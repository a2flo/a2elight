/*
 *  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2012 Florian Ziesche
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License only.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "shader.h"
#include <regex>

#define A2E_SHADER_LOG_SIZE 16384

/*! create and initialize the shader class
 */
shader::shader(engine* e_) {
	// get classes
	shader::e = e_;
	shader::f = e->get_file_io();
	shader::exts = e->get_ext();
	shader::r = e->get_rtt();
	shader::x = e->get_xml();
	
	a2e_shd = new a2e_shader(e);
	a2e_shd->set_shader_class(this);
	
	gui_shader_rendering = false;
	
	// load shader includes
	a2e_shd->load_a2e_shader_includes();

	// load/compile internal shaders
	if(!load_internal_shaders()) {
		a2e_error("there were problems loading/compiling the internal shaders!");
	}
	else {
		a2e_debug("internal shaders compiled successfully!");
	}
}

/*! delete everything
 */
shader::~shader() {
	a2e_debug("deleting shader object");
	
	delete a2e_shd;

	for(const auto& shd : shaders) {
		delete shd.second;
	}
	shaders.clear();

	a2e_debug("shader object deleted");
}

void shader::reload_shaders() {
	// delete old shaders
	if(a2e_shd != nullptr) delete a2e_shd;
	for(const auto& shd : shaders) {
		delete shd.second;
	}
	shaders.clear();

	// recreate
	a2e_shd = new a2e_shader(e);
	a2e_shd->set_shader_class(this);
	a2e_shd->load_a2e_shader_includes();

	// load/compile internal shaders
	if(!load_internal_shaders()) {
		a2e_error("there were problems loading/compiling the internal shaders!");
	}
	else {
		a2e_debug("internal shaders compiled successfully!");
	}
	
	// load/compile external shaders
	map<string, string> extshaders = external_shaders; // add_a2e_shader will modify external_shaders (-> invalid iter), so copy it
	size_t err_shd_cnt = 0;
	for(const auto& ext_shd : extshaders) {
		if(!add_a2e_shader(ext_shd.first, ext_shd.second)) err_shd_cnt++;
	}
	if(err_shd_cnt == 0) a2e_debug("external shaders compiled successfully!");
	else a2e_debug("failed to compile %u external!", err_shd_cnt);
	
	// emit shader reload event
	e->get_event()->add_event(EVENT_TYPE::SHADER_RELOAD, make_shared<shader_reload_event>(SDL_GetTicks()));
}

void shader::copy_buffer(rtt::fbo* src_buffer, rtt::fbo* dest_buffer, unsigned int src_attachment, unsigned int dest_attachment) {
#if !defined(A2E_IOS)
	if((src_buffer->target[src_attachment] != GL_TEXTURE_2D && src_buffer->target[src_attachment] != GL_TEXTURE_RECTANGLE) ||
	   (dest_buffer->target[dest_attachment] != GL_TEXTURE_2D && dest_buffer->target[dest_attachment] != GL_TEXTURE_RECTANGLE)) {
		a2e_error("non-2D buffers aren't allowed to be copied at the moment!");
	}
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_buffer->fbo_id);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+src_attachment, src_buffer->target[src_attachment], src_buffer->tex[src_attachment], 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0+src_attachment);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_buffer->fbo_id);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+dest_attachment, dest_buffer->target[dest_attachment], dest_buffer->tex[dest_attachment], 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0+dest_attachment);
	
	glBlitFramebuffer(0, 0, src_buffer->width, src_buffer->height,
					  0, 0, dest_buffer->width, dest_buffer->height,
					  GL_COLOR_BUFFER_BIT | ((src_buffer->depth_type != rtt::DT_NONE && dest_buffer->depth_type != rtt::DT_NONE) ? GL_DEPTH_BUFFER_BIT : 0),
					  GL_NEAREST);
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, A2E_DEFAULT_FRAMEBUFFER);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, A2E_DEFAULT_FRAMEBUFFER);
#else
	// TODO: implement framebuffer blitting on iOS
#endif
}

/*! adds a shader object
 */
shader_object* shader::add_shader_file(const string& identifier, ext::GLSL_VERSION glsl_version, const char* vname, const char* gname, const char* fname) {
	size_t size;
	shader_object* ret;
	char* vs_text = nullptr;
	char* gs_text = nullptr;
	char* fs_text = nullptr;
	
	// load shaders
	f->open(vname, file_io::OT_READ_BINARY);
	size = (size_t)f->get_filesize();
	vs_text = new char[size+1];
	f->get_block(vs_text, size);
	vs_text[size] = 0;
	f->close();
	
	// optionally load geometry shader ...
	if(gname != nullptr) {
		f->open(gname, file_io::OT_READ_BINARY);
		size = (size_t)f->get_filesize();
		gs_text = new char[size+1];
		f->get_block(gs_text, size);
		gs_text[size] = 0;
		f->close();
	}
	
	f->open(fname, file_io::OT_READ_BINARY);
	size = (size_t)f->get_filesize();
	fs_text = new char[size+1];
	f->get_block(fs_text, size);
	fs_text[size] = 0;
	f->close();
	
	// add shader
	ret = add_shader_src(identifier, glsl_version, vs_text, gs_text, fs_text);
	
	delete [] vs_text;
	delete [] gs_text;
	delete [] fs_text;
	
	return ret;
}

shader_object* shader::add_shader_src(const string& identifier, ext::GLSL_VERSION glsl_version, const char* vs_text, const char* gs_text, const char* fs_text) {
	return add_shader_src(identifier, "", glsl_version, vs_text, gs_text, fs_text);
}

/*! adds a shader object
 */
shader_object* shader::add_shader_src(const string& identifier, const string& option, ext::GLSL_VERSION glsl_version, const char* vs_text, const char* gs_text, const char* fs_text) {
	// success flag (if it's 1 (true), we successfully created a shader object)
	int success;
	GLchar info_log[A2E_SHADER_LOG_SIZE];
	
	if(gs_text != nullptr && strcmp(gs_text, "") == 0) gs_text = nullptr;
	
	// create a new shader object if none exists for this identifier
	if(shaders.count(identifier) == 0) {
		shaders[identifier] = new shader_object(identifier);
	}
	
	// add a new program object to this shader
	shaders[identifier]->programs.push_back(new shader_object::internal_shader_object());
	if(option != "") {
		shaders[identifier]->options[option] = shaders[identifier]->programs.back();
	}
	shader_object::internal_shader_object& shd_obj = *shaders[identifier]->programs.back();
	shaders[identifier]->glsl_version = std::max(shaders[identifier]->glsl_version, glsl_version);
	
	// create the vertex shader object
	shd_obj.vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shd_obj.vertex_shader, 1, (GLchar const**)&vs_text, nullptr);
	glCompileShader(shd_obj.vertex_shader);
	glGetShaderiv(shd_obj.vertex_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(shd_obj.vertex_shader, A2E_SHADER_LOG_SIZE, nullptr, info_log);
		a2e_error("Error in vertex shader \"%s/%s\" compilation!", identifier, option);
		log_pretty_print(info_log, vs_text);
		return 0;
	}
	
#if !defined(A2E_IOS)
	// create the geometry shader object
	if(gs_text != nullptr && strcmp(gs_text, "") != 0) {
		shd_obj.geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(shd_obj.geometry_shader, 1, (GLchar const**)&gs_text, nullptr);
		glCompileShader(shd_obj.geometry_shader);
		glGetShaderiv(shd_obj.geometry_shader, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(shd_obj.geometry_shader, A2E_SHADER_LOG_SIZE, nullptr, info_log);
			a2e_error("Error in geometry shader \"%s/%s\" compilation!", identifier, option);
			log_pretty_print(info_log, gs_text);
			return 0;
		}
	}
	else shd_obj.geometry_shader = 0;
#else
	if(gs_text != nullptr && strcmp(gs_text, "") != 0) {
		a2e_error("geometry shaders are not supported in OpenGL ES 2.0!");
	}
	shd_obj.geometry_shader = 0;
#endif
	
	// create the fragment shader object
	shd_obj.fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shd_obj.fragment_shader, 1, (GLchar const**)&fs_text, nullptr);
	glCompileShader(shd_obj.fragment_shader);
	glGetShaderiv(shd_obj.fragment_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(shd_obj.fragment_shader, A2E_SHADER_LOG_SIZE, nullptr, info_log);
		a2e_error("Error in fragment shader \"%s/%s\" compilation!", identifier, option);
		log_pretty_print(info_log, fs_text);
		return 0;
	}
	
	// create the program object
	shd_obj.program = glCreateProgram();
	// attach the vertex and fragment shader progam to it
	glAttachShader(shd_obj.program, shd_obj.vertex_shader);
	glAttachShader(shd_obj.program, shd_obj.fragment_shader);
	if(gs_text != nullptr) {
		glAttachShader(shd_obj.program, shd_obj.geometry_shader);
	}

	// WIP: program binary
#if defined(A2E_DEBUG_PROGRAM_BINARY)
#if defined(__APPLE__)
	{
		fstream progcode_vs("/tmp/a2e_shd_code_vs.glsl", fstream::out);
		progcode_vs << vs_text << endl;
		progcode_vs.close();
		if(shd_obj.geometry_shader != 0) {
			fstream progcode_gs("/tmp/a2e_shd_code_gs.glsl", fstream::out);
			progcode_gs << gs_text << endl;
			progcode_gs.close();
		}
		fstream progcode_fs("/tmp/a2e_shd_code_fs.glsl", fstream::out);
		progcode_fs << fs_text << endl;
		progcode_fs.close();
		
		string output_vs = "", output_gs = "", output_fs = "";
		core::system("cgc -profile gp4vp -strict -oglsl /tmp/a2e_shd_code_vs.glsl 2>&1", output_vs);
		if(shd_obj.geometry_shader != 0) {
			core::system("cgc -profile gp4gp -strict -oglsl -po POINT /tmp/a2e_shd_code_gs.glsl 2>&1", output_gs);
		}
		core::system("cgc -profile gp4fp -strict -oglsl /tmp/a2e_shd_code_fs.glsl 2>&1", output_fs);
		
		system("rm /tmp/a2e_shd_code_vs.glsl");
		if(shd_obj.geometry_shader != 0) {
			system("rm /tmp/a2e_shd_code_gs.glsl");
		}
		system("rm /tmp/a2e_shd_code_fs.glsl");
		
		//
		shader_debug::add(identifier, option, output_vs, output_gs, output_fs);
	}
#else
	if(exts->is_ext_supported("GL_ARB_get_program_binary")) glProgramParameteri(shd_obj.program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
#endif
#endif
	
	// now link the program object
	glLinkProgram(shd_obj.program);
	glGetProgramiv(shd_obj.program, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(shd_obj.program, A2E_SHADER_LOG_SIZE, nullptr, info_log);
		a2e_error("Error in program \"%s/%s\" linkage!\nInfo log: %s", identifier, option, info_log);
		return 0;
	}
	glUseProgram(shd_obj.program);
	
	// bind frag data locations (frag_color, frag_color_2, frag_color_3, ...)
	bool fd_relink = false;
#if !defined(A2E_IOS)
	const unsigned int max_draw_buffers = exts->get_max_draw_buffers();
	for(unsigned int i = 0; i < max_draw_buffers; i++) {
		string name = "frag_color";
		if(i >= 1) name += "_"+uint2string(i+1);
		const GLint location = glGetFragDataLocation(shd_obj.program, name.c_str());
		
		// check if the frag color exists and must be bound to a different location
		if(location >= 0 && i != (GLuint)location) {
			// if so, bind it to the correct location
			glBindFragDataLocation(shd_obj.program, i, name.c_str());
			fd_relink = true;
		}
	}
#else
	// TODO: what todo on iOS?
#endif
	if(fd_relink) {
		// program must be linked again after the frag data locations were modified
		// (double-linkage sucks, but there's no other way in opengl 3.2 ...)
		glLinkProgram(shd_obj.program);
	}
	
	// WIP: program binary
#if defined(A2E_DEBUG_PROGRAM_BINARY)
#if !defined(__APPLE__)
	if(exts->is_ext_supported("GL_ARB_get_program_binary")) {
		GLint binary_length = 0;
		glGetProgramiv(shd_obj.program, GL_PROGRAM_BINARY_LENGTH, &binary_length);

		unsigned char* binary = new unsigned char[binary_length];
		GLenum binary_format = 0;
		glGetProgramBinary(shd_obj.program, binary_length, nullptr, &binary_format, binary);

		string binary_fname = "shader_binary_"+identifier+"_"+size_t2string(shaders[identifier]->programs.size()-1)+".dat";
		f->open_file(binary_fname.c_str(), file_io::OT_WRITE_BINARY);
		f->write_block((const char*)binary, binary_length, false);
		f->close_file();

		delete [] binary;
	}
#endif
#endif
	
	// grab number and names of all attributes and uniforms and get their locations (needs to be done before validation, b/c we have to set sampler locations)
	GLint attr_count = 0, uni_count = 0, max_attr_len = 0, max_uni_len = 0;
	GLint var_location = 0;
	GLint var_size = 0;
	GLenum var_type = 0;
	
	glGetProgramiv(shd_obj.program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_attr_len);
	glGetProgramiv(shd_obj.program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uni_len);
	glGetProgramiv(shd_obj.program, GL_ACTIVE_ATTRIBUTES, &attr_count);
	glGetProgramiv(shd_obj.program, GL_ACTIVE_UNIFORMS, &uni_count);
	max_attr_len+=2;
	max_uni_len+=2;
	
#if !defined(A2E_IOS)
	GLint uni_block_count = 0, max_uni_block_len = 0;
	glGetProgramiv(shd_obj.program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &max_uni_block_len);
	glGetProgramiv(shd_obj.program, GL_ACTIVE_UNIFORM_BLOCKS, &uni_block_count);
	max_uni_block_len+=2;
#endif
		
	// note: this may report weird attribute/uniform names (and locations), if uniforms/attributes are optimized away by the compiler
	
	bool print_debug_info = false;
	//if(identifier == "SIMPLE") {
	/*if(identifier == "FONT") {
		print_debug_info = true;
		cout << endl << "## " << identifier << "::" << option << endl;
	}*/
	
	GLchar* attr_name = new GLchar[max_attr_len];
	if(print_debug_info) a2e_log("## shader: %s", identifier);
	if(print_debug_info) a2e_log("GL_ACTIVE_ATTRIBUTES: %u", attr_count);
	for(GLint attr = 0; attr < attr_count; attr++) {
		memset(attr_name, 0, max_attr_len);
		glGetActiveAttrib(shd_obj.program, attr, max_attr_len-1, nullptr, &var_size, &var_type, attr_name);
		var_location = glGetAttribLocation(shd_obj.program, attr_name);
		if(var_location < 0) {
			if(print_debug_info) a2e_error("Warning: could not get location for attribute \"%s\" in shader #%s/%s!", attr_name, identifier, option);
			continue;
		}
		if(print_debug_info) a2e_log("attribute #%u: %s", var_location, attr_name);
		
		string attribute_name = attr_name;
		if(attribute_name.find("[") != string::npos) attribute_name = attribute_name.substr(0, attribute_name.find("["));
		shd_obj.attributes.insert(make_pair(attribute_name,
											shader_object::internal_shader_object::shader_variable(var_location, var_size, var_type)));
	}
	delete [] attr_name;
	
	GLchar* uni_name = new GLchar[max_uni_len];
	if(print_debug_info) a2e_log("GL_ACTIVE_UNIFORMS: %u", uni_count);
	for(GLint uniform = 0; uniform < uni_count; uniform++) {
		memset(uni_name, 0, max_uni_len);
		glGetActiveUniform(shd_obj.program, uniform, max_uni_len-1, nullptr, &var_size, &var_type, uni_name);
		var_location = glGetUniformLocation(shd_obj.program, uni_name);
		if(var_location < 0) {
			if(print_debug_info) a2e_error("Warning: could not get location for uniform \"%s\" in shader #%s/%s!", uni_name, identifier, option);
			continue;
		}
		if(print_debug_info) a2e_log("uniform #%u: %s", var_location, uni_name);
		string uniform_name = uni_name;
		if(uniform_name.find("[") != string::npos) uniform_name = uniform_name.substr(0, uniform_name.find("["));
		shd_obj.uniforms.insert(make_pair(uniform_name,
										  shader_object::internal_shader_object::shader_variable(var_location, var_size, var_type)));
		
		// if the uniform is a sampler, add it to the sampler mapping (with increasing id/num)
		// also: use shader_gl3 here, because we can't use shader_base directly w/o instantiating it
#if !defined(A2E_IOS)
		if(shader_gl3::is_gl_sampler_type(var_type)) {
#else
		if(shader_gles2::is_gl_sampler_type(var_type)) {
#endif
			shd_obj.samplers.insert(make_pair(uniform_name, shd_obj.samplers.size()));
			
			// while we are at it, also set the sampler location to a dummy value (this has to be done to satisfy program validation)
			glUniform1i(var_location, (GLint)shd_obj.samplers.size()-1);
		}
	}
	delete [] uni_name;
	
#if !defined(A2E_IOS)
	GLchar* uni_block_name = new GLchar[max_uni_block_len];
	if(print_debug_info) a2e_log("GL_ACTIVE_UNIFORM_BLOCKS: %u", uni_block_count);
	for(GLint block = 0; block < uni_block_count; block++) {
		memset(uni_block_name, 0, max_uni_block_len);
		glGetActiveUniformBlockName(shd_obj.program, block, max_uni_block_len-1, nullptr, uni_block_name);
		
		GLuint block_index = glGetUniformBlockIndex(shd_obj.program, uni_block_name);
		if(block_index == GL_INVALID_INDEX) {
			if(print_debug_info) a2e_error("Warning: could not get index for uniform block \"%s\" in shader #%s/%s!", uni_block_name, identifier, option);
			continue;
		}
		
		GLint data_size = 0;
		glGetActiveUniformBlockiv(shd_obj.program, block_index, GL_UNIFORM_BLOCK_DATA_SIZE, &data_size);
		if(print_debug_info) a2e_log("uniform block #%u (size: %u): %s",
									 block_index, data_size, uni_block_name);
		const string uniform_block_name = uni_block_name;
		
		shd_obj.blocks.insert(make_pair(uniform_block_name,
										shader_object::internal_shader_object::shader_variable(block_index, data_size, GL_UNIFORM_BUFFER)));
		
		// TODO: handle samplers?
	}
	delete [] uni_block_name;
#endif
	
	// validate the program object
	glValidateProgram(shd_obj.program);
	glGetProgramiv(shd_obj.program, GL_VALIDATE_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(shd_obj.program, A2E_SHADER_LOG_SIZE, nullptr, info_log);
		a2e_error("Error in program \"%s/%s\" validation!\nInfo log: %s", identifier, option, info_log);
		return 0;
	}
	else {
		glGetProgramInfoLog(shd_obj.program, A2E_SHADER_LOG_SIZE, nullptr, info_log);
		
		// check if shader will run in software (if so, print out a debug message)
		if(strstr((const char*)info_log, (const char*)"software") != nullptr) {
			a2e_debug("program \"%s/%s\" validation: %s", identifier, option, info_log);
		}
	}
	
	//
	glUseProgram(0);

	return shaders[identifier];
}

#if defined(__APPLE__)
void shader::log_pretty_print(const char* log, const char* code) const {
	static const regex rx_log_line("\\w+: 0:(\\d+):.*");
	smatch regex_result;
	
	vector<string> lines = core::tokenize(string(log), '\n');
	vector<string> code_lines = core::tokenize(string(code), '\n');
	for(const string& line : lines) {
		if(line.size() == 0) continue;
		a2e_log("## \033[31m%s\\E[m", line);
		
		// find code line and print it (+/- 1 line)
		if(regex_match(line, regex_result, rx_log_line)) {
			const size_t src_line_num = string2size_t(regex_result[1]) - 1;
			if(src_line_num < code_lines.size()) {
				if(src_line_num != 0) {
					a2e_log("\033[37m%s\\E[m", code_lines[src_line_num-1]);
				}
				a2e_log("\033[31m%s\\E[m", code_lines[src_line_num]);
				if(src_line_num+1 < code_lines.size()) {
					a2e_log("\033[37m%s\\E[m", code_lines[src_line_num+1]);
				}
			}
			a2e_log("");
		}
	}
}
#else
void shader::log_pretty_print(const char* log, const char* code a2e_unused) const {
	a2e_log("Info Log: %s", log);
}
#endif

shader_object* shader::get_shader_object(const string& identifier) {
	return shaders[identifier];
}

bool shader::load_internal_shaders() {
	bool ret = true;
	bool err_cur_shader = false;
	
	//
	static const string internal_shaders[][2] = {
		// misc/postprocess/hdr/...
		{ "BLURLINE3", "misc/blurline3.a2eshd" },
		{ "BLURLINE5", "misc/blurline5.a2eshd" },
#if !defined(A2E_IOS)
		{ "LUMA", "misc/luma.a2eshd" },
		{ "FXAA", "misc/fxaa3.a2eshd" },
#endif
		{ "SIMPLE", "misc/simple.a2eshd" }, // basically a replacement for fixed-function shading/drawing
		//"DOWNSAMPLE", "misc/downsample.a2eshd" },
		{ "BLEND", "misc/blend.a2eshd" },
		
		// gfx2d
		{ "GFX2D_GRADIENT", "gfx2d/gradient.a2eshd" },
		{ "GFX2D_TEXTURE", "gfx2d/texture.a2eshd" },
		{ "FONT", "gfx2d/font.a2eshd" },

		// inferred rendering
		{ "IR_GP_GBUFFER", "inferred/gp_gbuffer.a2eshd" },
		{ "IR_GP_GBUFFER_PARALLAX", "inferred/gp_gbuffer_parallax.a2eshd" },
		{ "IR_LP_STENCIL", "inferred/lp_stencil.a2eshd" },
		{ "IR_LP_PHONG", "inferred/lp_phong.a2eshd" },
		{ "IR_LP_ASHIKHMIN_SHIRLEY", "inferred/lp_ashikhmin_shirley.a2eshd" },
		{ "IR_MP_DIFFUSE", "inferred/mp_diffuse.a2eshd" },
		{ "IR_MP_PARALLAX", "inferred/mp_parallax.a2eshd" },
		
#if !defined(A2E_IOS)
		// particle
		{ "PARTICLE_DRAW_OPENCL", "particle/particle_draw_ocl.a2eshd" },
#endif
	};
	
	for(const auto& int_shader : internal_shaders) {
		err_cur_shader = false;
		const string& shader_identifier = int_shader[0];
		const string& shader_filename = int_shader[1];
		
		if(shader_filename != "") {
			a2e_shd->add_a2e_shader(shader_identifier);
			if(!a2e_shd->load_a2e_shader(shader_identifier, e->shader_path(shader_filename.c_str()), a2e_shd->get_a2e_shader(shader_identifier))) {
				err_cur_shader = true;
				ret = false;
			}
			else {
				if(!a2e_shd->process_and_compile_a2e_shader(a2e_shd->get_a2e_shader(shader_identifier))) {
					err_cur_shader = true;
					ret = false;
				}
			}
		}
		else {
			err_cur_shader = true;
		}
		
		if(err_cur_shader) a2e_error("error loading shader file \"%s\"!", shader_filename.c_str());
	}
	
	return ret;
}

void shader::set_gui_shader_rendering(bool state) {
	gui_shader_rendering = state;
}

bool shader::is_gui_shader_rendering() {
	return gui_shader_rendering;
}

a2e_shader* shader::get_a2e_shader() {
	return a2e_shd;
}

bool shader::add_a2e_shader(const string& identifier, const string& filename) {
	a2e_shd->add_a2e_shader(identifier);
	if(!a2e_shd->load_a2e_shader(identifier, e->shader_path(filename.c_str()), a2e_shd->get_a2e_shader(identifier))) {
		a2e_error("couldn't load a2e-shader \"%s\"!", filename);
		return false;
	}
	else {
		if(!a2e_shd->process_and_compile_a2e_shader(a2e_shd->get_a2e_shader(identifier))) {
			a2e_error("couldn't process and/or compile a2e-shader \"%s\"!", filename);
			return false;
		}
	}
	external_shaders[identifier] = filename;
	return true;
}

////
#define A2E_SHADER_IMPL_TO_STR(x) #x
#define make_get_shader(ret_type, shader_impl, min_glsl, max_glsl) \
template <> ret_type shader::get_shader(const string& identifier) const { \
	if(shaders.count(identifier) == 0) return nullptr; \
	 \
	const shader_object& shd_obj = *shaders.find(identifier)->second; \
	if(shd_obj.glsl_version < min_glsl || shd_obj.glsl_version > max_glsl) { \
		a2e_error("requested gl type \"%s\" doesn't match shader \"%s\" glsl version %s!", \
				 A2E_SHADER_IMPL_TO_STR(shader_impl), identifier, exts->cstr_from_glsl_version(shd_obj.glsl_version)); \
		return nullptr; \
	} \
	 \
	return make_shared<shader_impl>(shd_obj); \
} \
ret_type shader::get_##ret_type(const string& identifier) const { return get_shader<ret_type>(identifier); }

#if !defined(A2E_IOS)
make_get_shader(gl3shader, shader_gl3, ext::GLSL_150, ext::GLSL_330);
#else
make_get_shader(gl3shader, shader_gles2, ext::GLSL_ES_100, ext::GLSL_ES_100);
#endif
