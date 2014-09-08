/*
 *  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2014 Florian Ziesche
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

#include "shader_gl3.hpp"

#if !defined(FLOOR_IOS)

#if defined(A2E_DEBUG) // type checking in debug mode

#define __A2E_DECLARE_GL_TYPE_TO_STRING(type) case type: return #type;

const char* gl3_type_to_string(const size_t& type); // prototype ...
const char* gl3_type_to_string(const size_t& type) {
	switch(type) {
			A2E_GL_SHADER_TYPES(__A2E_DECLARE_GL_TYPE_TO_STRING);
		default: break;
	}
	stringstream buffer;
	buffer << hex << uppercase << (size_t)type;
	return buffer.str().c_str();
}

#define A2E_CHECK_UNIFORM_EXISTENCE(name) \
if(shd_obj.programs.size() <= cur_program) { \
log_error("invalid program #%u for shader \"%s\"!", cur_program, shd_obj.name.c_str()); \
return; \
} \
if(shd_obj.programs[cur_program]->uniforms.count(name) == 0) { \
log_error("unknown uniform name \"%s\" for shader \"%s\"!", name, shd_obj.name.c_str()); \
return; \
}

#define A2E_CHECK_ATTRIBUTE_EXISTENCE(name) \
if(shd_obj.programs.size() <= cur_program) { \
log_error("invalid program #%u for shader \"%s\"!", cur_program, shd_obj.name.c_str()); \
return; \
} \
if(shd_obj.programs[cur_program]->attributes.count(name) == 0) { \
log_error("unknown attribute name \"%s\" for shader \"%s\"!", name, shd_obj.name.c_str()); \
return; \
}

#define A2E_CHECK_BLOCK_EXISTENCE(name) \
if(shd_obj.programs.size() <= cur_program) { \
log_error("invalid program #%u for shader \"%s\"!", cur_program, shd_obj.name.c_str()); \
return; \
} \
if(shd_obj.programs[cur_program]->blocks.count(name) == 0) { \
log_error("unknown uniform block name \"%s\" for shader \"%s\"!", name, shd_obj.name.c_str()); \
return; \
}

#define A2E_CHECK_UNIFORM_TYPE(name, uniform_type) \
size_t expected_type = shd_obj.programs[cur_program]->uniforms.find(name)->second.type; \
if(uniform_type != (size_t)expected_type) { \
log_error("unexpected type %s for uniform \"%s\" - expected %s (in shader \"%s\")!", gl3_type_to_string(uniform_type), name, gl3_type_to_string(expected_type), shd_obj.name.c_str()); \
}

#define A2E_CHECK_ATTRIBUTE_TYPE(name, attribute_type) \
size_t expected_type = shd_obj.programs[cur_program]->attributes.find(name)->second.type; \
if(attribute_type != (size_t)expected_type) { \
log_error("unexpected type %s for attribute \"%s\" - expected %s (in shader \"%s\")!", gl3_type_to_string(attribute_type), name, gl3_type_to_string(expected_type), shd_obj.name.c_str()); \
}

#else // don't check the type in release mode
#define A2E_CHECK_UNIFORM_EXISTENCE(name)
#define A2E_CHECK_ATTRIBUTE_EXISTENCE(name)
#define A2E_CHECK_BLOCK_EXISTENCE(name)
#define A2E_CHECK_UNIFORM_TYPE(name, uniform_type)
#define A2E_CHECK_ATTRIBUTE_TYPE(name, attribute_type)
#endif

///////////////////////////////////////////////////////////////////////////////////////
// shader_gl3 functions
shader_gl3::shader_gl3(const shader_object& shd_obj_) : shader_base<shader_gl3>(shd_obj_) {
	use(0);
#if defined(A2E_DEBUG)
	if(shd_obj.programs.size() == 0) {
		log_error("shader \"%s\" has no programs!", shd_obj.name.c_str());
	}
#endif
}

void shader_gl3::disable() {
	// disable all set vertex attribute arrays
	for(const auto& vattr_iter : active_vertex_attribs) {
		glDisableVertexAttribArray((GLuint)vattr_iter);
	}
	if(!active_vertex_attribs.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	active_vertex_attribs.clear();
	
	// unbind ubo
	if(shd_obj.programs[cur_program]->blocks.size() > 0) {
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	
	// disable program
	glUseProgram(0);
}

void shader_gl3::use() {
	shader_gl3::use(0);
}

void shader_gl3::use(const size_t& program) {
	shader_base::use(program);
	glUseProgram(shd_obj.programs[cur_program]->program);
#if defined(A2E_DEBUG)
	if(shd_obj.programs.size() == 0) {
		log_error("no program #%u exists in shader \"%s\"!", program, shd_obj.name.c_str());
	}
#endif
}

void shader_gl3::use(const string& option, const set<string> combiners) {
	const string combined_option(option + accumulate(cbegin(combiners), cend(combiners), string(),
													 [](string& ret, const string& in) {
														 return ret + in;
													 }));
#if defined(A2E_DEBUG)
	if(shd_obj.options.count(combined_option) == 0) {
		log_error("no option \"%s\" exists in shader \"%s\"!", combined_option, shd_obj.name);
		return;
	}
#endif
	cur_option = combined_option;
	const shader_object::internal_shader_object* int_shd_obj = shd_obj.options.find(cur_option)->second;
	const auto piter = find(shd_obj.programs.cbegin(), shd_obj.programs.cend(), int_shd_obj);
	cur_program = (size_t)distance(shd_obj.programs.begin(), piter);
	use(cur_program);
}

size_t shader_gl3::get_cur_program() const {
	return cur_program;
}

const string& shader_gl3::get_cur_option() const {
	return cur_option;
}

///////////////////////////////////////////////////////////////////////////////////////
// -> uniform

// 1{i,ui,f,b,fv,iv,uiv,bv}
void shader_gl3::uniform(const char* name, const float& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT);
	glUniform1f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1);
}

void shader_gl3::uniform(const char* name, const int& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT);
	glUniform1i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1);
}

void shader_gl3::uniform(const char* name, const unsigned int& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT);
	glUniform1ui(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1);
}

void shader_gl3::uniform(const char* name, const bool& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL);
	glUniform1i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1);
}

void shader_gl3::uniform(const char* name, const float* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT);
	glUniform1fv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, arg1);
}

void shader_gl3::uniform(const char* name, const int* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT);
	glUniform1iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, arg1);
}

void shader_gl3::uniform(const char* name, const unsigned int* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT);
	glUniform1uiv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, arg1);
}

void shader_gl3::uniform(const char* name, const bool* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL);
	GLint* int_array = new int[count];
	for(size_t i = 0; i < count; i++) { int_array[i] = arg1[i]; }
	glUniform1iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, int_array);
	delete [] int_array;
}

// 2{i,ui,f,b,fv,iv,uiv,bv}
void shader_gl3::uniform(const char* name, const float& arg1, const float& arg2) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC2);
	glUniform2f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2);
}

void shader_gl3::uniform(const char* name, const int& arg1, const int& arg2) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC2);
	glUniform2i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2);
}

void shader_gl3::uniform(const char* name, const unsigned int& arg1, const unsigned int& arg2) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT_VEC2);
	glUniform2ui(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2);
}

void shader_gl3::uniform(const char* name, const bool& arg1, const bool& arg2) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC2);
	glUniform2i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2);
}

void shader_gl3::uniform(const char* name, const float2& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC2);
	glUniform2f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y);
}

void shader_gl3::uniform(const char* name, const int2& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC2);
	glUniform2i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y);
}

void shader_gl3::uniform(const char* name, const uint2& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT_VEC2);
	glUniform2ui(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y);
}

void shader_gl3::uniform(const char* name, const bool2& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC2);
	glUniform2i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y);
}

void shader_gl3::uniform(const char* name, const float2* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC2);
	glUniform2fv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLfloat*)arg1);
}

void shader_gl3::uniform(const char* name, const int2* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC2);
	glUniform2iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLint*)arg1);
}

void shader_gl3::uniform(const char* name, const uint2* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT_VEC2);
	glUniform2uiv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLuint*)arg1);
}

void shader_gl3::uniform(const char* name, const bool2* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC2);
	GLint* int_array = new int[count*2];
	for(size_t i = 0; i < (count*2); i++) {
		int_array[i] = arg1[i].x;
		i++;
		int_array[i] = arg1[i].y;
	}
	glUniform1iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, int_array);
	delete [] int_array;
}


// 3{i,ui,f,b,fv,iv,uiv,bv}
void shader_gl3::uniform(const char* name, const float& arg1, const float& arg2, const float& arg3) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC3);
	glUniform3f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3);
}

void shader_gl3::uniform(const char* name, const int& arg1, const int& arg2, const int& arg3) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC3);
	glUniform3i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3);
}

void shader_gl3::uniform(const char* name, const unsigned int& arg1, const unsigned int& arg2, const unsigned int& arg3) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT_VEC3);
	glUniform3ui(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3);
}

void shader_gl3::uniform(const char* name, const bool& arg1, const bool& arg2, const bool& arg3) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC3);
	glUniform3i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3);
}

void shader_gl3::uniform(const char* name, const float3& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC3);
	glUniform3f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z);
}

void shader_gl3::uniform(const char* name, const int3& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC3);
	glUniform3i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z);
}

void shader_gl3::uniform(const char* name, const uint3& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT_VEC3);
	glUniform3ui(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z);
}

void shader_gl3::uniform(const char* name, const bool3& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC3);
	glUniform3i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z);
}

void shader_gl3::uniform(const char* name, const float3* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC3);
	glUniform3fv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLfloat*)arg1);
}

void shader_gl3::uniform(const char* name, const int3* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC3);
	glUniform3iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLint*)arg1);
}

void shader_gl3::uniform(const char* name, const uint3* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT_VEC3);
	glUniform3uiv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLuint*)arg1);
}

void shader_gl3::uniform(const char* name, const bool3* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC3);
	GLint* int_array = new int[count*3];
	for(size_t i = 0; i < (count*3); i++) {
		int_array[i] = arg1[i].x;
		i++;
		int_array[i] = arg1[i].y;
	}
	glUniform3iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, int_array);
	delete [] int_array;
}


// 4{i,ui,f,b,fv,iv,uiv,bv}
void shader_gl3::uniform(const char* name, const float& arg1, const float& arg2, const float& arg3, const float& arg4) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC4);
	glUniform4f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gl3::uniform(const char* name, const int& arg1, const int& arg2, const int& arg3, const int& arg4) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC4);
	glUniform4i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gl3::uniform(const char* name, const unsigned int& arg1, const unsigned int& arg2, const unsigned int& arg3, const unsigned int& arg4) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT_VEC4);
	glUniform4ui(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gl3::uniform(const char* name, const bool& arg1, const bool& arg2, const bool& arg3, const bool& arg4) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC4);
	glUniform4i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gl3::uniform(const char* name, const float4& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC4);
	glUniform4f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z, arg1.w);
}

void shader_gl3::uniform(const char* name, const int4& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC4);
	glUniform4i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z, arg1.w);
}

void shader_gl3::uniform(const char* name, const uint4& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT_VEC4);
	glUniform4ui(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z, arg1.w);
}

void shader_gl3::uniform(const char* name, const bool4& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC4);
	glUniform4i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z, arg1.w);
}

void shader_gl3::uniform(const char* name, const float4* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC4);
	glUniform4fv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLfloat*)arg1);
}

void shader_gl3::uniform(const char* name, const int4* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC4);
	glUniform4iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLint*)arg1);
}

void shader_gl3::uniform(const char* name, const uint4* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_UNSIGNED_INT_VEC4);
	glUniform4uiv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLuint*)arg1);
}

void shader_gl3::uniform(const char* name, const bool4* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC4);
	GLint* int_array = new int[count*4];
	for(size_t i = 0; i < (count*4); i++) {
		int_array[i] = arg1[i].x;
		i++;
		int_array[i] = arg1[i].y;
	}
	glUniform4iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, int_array);
	delete [] int_array;
}

// mat{--3,4}
void shader_gl3::uniform(const char* name, const matrix4f& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_MAT4);
	glUniformMatrix4fv(A2E_SHADER_GET_UNIFORM_POSITION(name), 1, false, (GLfloat*)&arg1.data[0]);
}

void shader_gl3::uniform(const char* name, const matrix4f* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_MAT4);
	glUniformMatrix4fv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, false, (GLfloat*)arg1);
}

///////////////////////////////////////////////////////////////////////////////////////
// -> texture

void shader_gl3::set_texture(const char* name, const GLuint& tex, const GLenum& texture_type) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
#if defined(A2E_DEBUG)
	// check texture number (0 == uninitialized)
	if(tex == 0) {
		log_error("invalid texture number %u for texture uniform \"%s\" (in shader \"%s\")!", tex, name, shd_obj.name.c_str());
	}
	// check type
	const size_t uniform_type = shd_obj.programs[cur_program]->uniforms.find(name)->second.type;
	if(!is_gl_sampler_type((const GLenum)uniform_type)) {
		log_error("unexpected type %s for texture uniform \"%s\" - expected a sampler type (in shader \"%s\")!", gl3_type_to_string(uniform_type), name, shd_obj.name.c_str());
	}
	// check sampler mapping existence
	if(shd_obj.programs[cur_program]->samplers.count(name) == 0) {
		log_error("no sampler mapping for texture uniform \"%s\" exists (in shader \"%s\")!", name, shd_obj.name.c_str());
	}
#endif
	
	const size_t tex_num = shd_obj.programs[cur_program]->samplers.find(name)->second;
#if defined(A2E_DEBUG)
	if(tex_num >= 32) {
		log_error("invalid texture number #%u for texture uniform \"%s\" - only 32 textures are allowed (in shader \"%s\")!", tex_num, name, shd_obj.name.c_str());
	}
#endif
	
	// set uniform, activate and bind texture
	glUniform1i(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLint)tex_num);
	glActiveTexture(GL_TEXTURE0 + (GLenum)tex_num);
	glBindTexture(texture_type, tex);
}

void shader_gl3::texture(const char* name, const GLuint& tex, const GLenum texture_type) const {
	set_texture(name, tex, texture_type);
}

void shader_gl3::texture(const char* name, const a2e_texture& tex) const {
	set_texture(name, tex->tex_num, tex->texture_type);
}

///////////////////////////////////////////////////////////////////////////////////////
// -> attribute

// NOTE: in opengl 2.x attributes types must be of type float! => overwrite these functions in opengl 3.x+

// 1{f,d,s,fv,dv,sv}
void shader_gl3::attribute(const char* name, const float& arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT);
	glVertexAttrib1f(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1);
}

void shader_gl3::attribute(const char* name, const double& arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT);
	glVertexAttrib1d(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1);
}

void shader_gl3::attribute(const char* name, const short& arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT);
	glVertexAttrib1s(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1);
}

void shader_gl3::attribute(const char* name, const float* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT);
	glVertexAttrib1fv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1);
}

void shader_gl3::attribute(const char* name, const double* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT);
	glVertexAttrib1dv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1);
}

void shader_gl3::attribute(const char* name, const short* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT);
	glVertexAttrib1sv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1);
}

// 2{f,d,s,fv,dv,sv}
void shader_gl3::attribute(const char* name, const float& arg1, const float& arg2) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC2);
	glVertexAttrib2f(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2);
}

void shader_gl3::attribute(const char* name, const double& arg1, const double& arg2) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC2);
	glVertexAttrib2d(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2);
}

void shader_gl3::attribute(const char* name, const short& arg1, const short& arg2) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC2);
	glVertexAttrib2s(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2);
}

void shader_gl3::attribute(const char* name, const float2* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC2);
	glVertexAttrib2fv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLfloat*)arg1);
}

void shader_gl3::attribute(const char* name, const double2* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC2);
	glVertexAttrib2dv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLdouble*)arg1);
}

void shader_gl3::attribute(const char* name, const short2* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC2);
	glVertexAttrib2sv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLshort*)arg1);
}

// 3{f,d,s,fv,dv,sv}
void shader_gl3::attribute(const char* name, const float& arg1, const float& arg2, const float& arg3) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC3);
	glVertexAttrib3f(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2, arg3);
}

void shader_gl3::attribute(const char* name, const double& arg1, const double& arg2, const double& arg3) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC3);
	glVertexAttrib3d(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2, arg3);
}

void shader_gl3::attribute(const char* name, const short& arg1, const short& arg2, const short& arg3) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC3);
	glVertexAttrib3s(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2, arg3);
}

void shader_gl3::attribute(const char* name, const float3* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC3);
	glVertexAttrib3fv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLfloat*)arg1);
}

/*void shader_gl3::attribute(const char* name, const double3* arg1) const {
 A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
 A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC3);
 glVertexAttrib3dv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLdouble*)arg1);
 }*/

void shader_gl3::attribute(const char* name, const short3* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC3);
	glVertexAttrib3sv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLshort*)arg1);
}

// 4{f,d,s,fv,dv,sv}
void shader_gl3::attribute(const char* name, const float& arg1, const float& arg2, const float& arg3, const float& arg4) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4f(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gl3::attribute(const char* name, const double& arg1, const double& arg2, const double& arg3, const double& arg4) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4d(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gl3::attribute(const char* name, const short& arg1, const short& arg2, const short& arg3, const short& arg4) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4s(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gl3::attribute(const char* name, const float4* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4fv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLfloat*)arg1);
}

void shader_gl3::attribute(const char* name, const double4* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4dv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLdouble*)arg1);
}

void shader_gl3::attribute(const char* name, const short4* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4sv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLshort*)arg1);
}

void shader_gl3::attribute(const char* name, const int4* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4iv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLint*)arg1);
}

void shader_gl3::attribute(const char* name, const char4* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4bv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLbyte*)arg1);
}

void shader_gl3::attribute(const char* name, const uchar4* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4ubv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLubyte*)arg1);
}

void shader_gl3::attribute(const char* name, const ushort4* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4usv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLushort*)arg1);
}

void shader_gl3::attribute(const char* name, const uint4* arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4uiv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLuint*)arg1);
}

///////////////////////////////////////////////////////////////////////////////////////
// -> attribute array

void shader_gl3::attribute_array(const char* name, const GLuint& buffer, const GLint& size, const GLenum type, const GLboolean normalized, const GLsizei stride) {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	
	// SHADER TODO: type/size via shader obj?
	
	const size_t location = A2E_SHADER_GET_ATTRIBUTE_POSITION(name);
	active_vertex_attribs.insert(location);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray((GLuint)location);
	switch(type) {
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_INT:
		case GL_UNSIGNED_INT:
			if(size <= 4) {
				glVertexAttribIPointer((GLuint)location, size, type, stride, nullptr);
			}
			else if(size == 9) {
				glVertexAttribIPointer((GLuint)location, 3, type, 36, nullptr);
				glEnableVertexAttribArray((GLuint)location+1);
				glVertexAttribIPointer((GLuint)location+1, 3, type, 36, (void*)12);
				glEnableVertexAttribArray((GLuint)location+2);
				glVertexAttribIPointer((GLuint)location+2, 3, type, 36, (void*)24);
			}
			else if(size == 16) {
				glVertexAttribIPointer((GLuint)location, 4, type, 64, nullptr);
				glEnableVertexAttribArray((GLuint)location+1);
				glVertexAttribIPointer((GLuint)location+1, 4, type, 64, (void*)16);
				glEnableVertexAttribArray((GLuint)location+2);
				glVertexAttribIPointer((GLuint)location+2, 4, type, 64, (void*)32);
				glEnableVertexAttribArray((GLuint)location+3);
				glVertexAttribIPointer((GLuint)location+3, 4, type, 64, (void*)48);
			}
			break;
		default:
			if(size <= 4) {
				glVertexAttribPointer((GLuint)location, size, type, normalized, stride, nullptr);
			}
			else if(size == 9) {
				glVertexAttribPointer((GLuint)location, 3, type, normalized, 36, nullptr);
				glEnableVertexAttribArray((GLuint)location+1);
				glVertexAttribPointer((GLuint)location+1, 3, type, normalized, 36, (void*)12);
				glEnableVertexAttribArray((GLuint)location+2);
				glVertexAttribPointer((GLuint)location+2, 3, type, normalized, 36, (void*)24);
			}
			else if(size == 16) {
				glVertexAttribPointer((GLuint)location, 4, type, normalized, 64, nullptr);
				glEnableVertexAttribArray((GLuint)location+1);
				glVertexAttribPointer((GLuint)location+1, 4, type, normalized, 64, (void*)16);
				glEnableVertexAttribArray((GLuint)location+2);
				glVertexAttribPointer((GLuint)location+2, 4, type, normalized, 64, (void*)32);
				glEnableVertexAttribArray((GLuint)location+3);
				glVertexAttribPointer((GLuint)location+3, 4, type, normalized, 64, (void*)48);
			}
			break;
	}
	// SHADER TODO: support for other matrix types
}

///////////////////////////////////////////////////////////////////////////////////////
// -> uniform buffer

void shader_gl3::block(const char* name, const GLuint& ubo) const {
	A2E_CHECK_BLOCK_EXISTENCE(name);
	
	const size_t index = A2E_SHADER_GET_BLOCK_POSITION(name);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBindBufferBase(GL_UNIFORM_BUFFER, (GLuint)index, ubo);
}

#endif
