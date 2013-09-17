/*  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2013 Florian Ziesche
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License only
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "engine.hpp"
#include "a2e_version.hpp"
#include "rendering/shader.hpp"
#include "cl/opencl.hpp"
#include "gui/gui.hpp"
#include "rendering/gfx2d.hpp"
#include "scene/scene.hpp"
#include "rendering/gl_timer.hpp"

#if defined(__APPLE__)
#include "osx/osx_helper.hpp"
#endif

// dll main for windows dll export
#if defined(__WINDOWS__)
BOOL APIENTRY DllMain(HANDLE hModule floor_unused, DWORD ul_reason_for_call, LPVOID lpReserved floor_unused);
BOOL APIENTRY DllMain(HANDLE hModule floor_unused, DWORD ul_reason_for_call, LPVOID lpReserved floor_unused) {
	switch(ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}
#endif // __WINDOWS__

void engine::init(const char* callpath_, const char* datapath_,
				  const bool console_only_, const string config_name_,
				  const char* ico_) {
	floor::init(callpath_, datapath_, console_only_, config_name_);
	floor::set_caption("A2E");
	
	// print out a2elight info
	log_debug("%s", (A2E_VERSION_STRING).c_str());
	
	// TODO: kernels?
	
	shaderpath = "shader/";
	standard_cursor = nullptr;
	cursor_data = nullptr;
	cursor_mask = nullptr;
	global_vao = 0;
	
	evt = floor::get_event();
	window_handler = new event::handler(this, &engine::window_event_handler);
	evt->add_internal_event_handler(*window_handler, EVENT_TYPE::WINDOW_RESIZE);
	
	// print out engine info
	log_debug("%s", (A2E_VERSION_STRING).c_str());
	
	// load config (that aren't already part of floor)
	const auto& config_doc = floor::get_config_doc();
	if(config_doc.valid) {
		// ui anti-aliasing should at least be 2x msaa
		config.ui_anti_aliasing = std::max(config_doc.get<size_t>("config.gui.anti_aliasing", 8), (size_t)2);
		
		config.key_repeat = config_doc.get<size_t>("config.input.key_repeat", 200);
		config.ldouble_click_time = config_doc.get<size_t>("config.input.ldouble_click_time", 200);
		config.mdouble_click_time = config_doc.get<size_t>("config.input.mdouble_click_time", 200);
		config.rdouble_click_time = config_doc.get<size_t>("config.input.rdouble_click_time", 200);
		
		config.fps_limit = config_doc.get<size_t>("config.sleep.time", 0);
		
		string filtering_str = config_doc.get<string>("config.graphic.filtering", "");
		if(filtering_str == "POINT") config.filtering = TEXTURE_FILTERING::POINT;
		else if(filtering_str == "LINEAR") config.filtering = TEXTURE_FILTERING::LINEAR;
		else if(filtering_str == "BILINEAR") config.filtering = TEXTURE_FILTERING::BILINEAR;
		else if(filtering_str == "TRILINEAR") config.filtering = TEXTURE_FILTERING::TRILINEAR;
		else config.filtering = TEXTURE_FILTERING::POINT;
		
		config.anisotropic = config_doc.get<size_t>("config.graphic.anisotropic", 0);
		
		string anti_aliasing_str = config_doc.get<string>("config.graphic.anti_aliasing", "");
		if(anti_aliasing_str == "NONE") config.anti_aliasing = rtt::TEXTURE_ANTI_ALIASING::NONE;
		else if(anti_aliasing_str == "FXAA") config.anti_aliasing = rtt::TEXTURE_ANTI_ALIASING::FXAA;
		else if(anti_aliasing_str == "2xSSAA") config.anti_aliasing = rtt::TEXTURE_ANTI_ALIASING::SSAA_2;
		//else if(anti_aliasing_str == "4xSSAA") config.anti_aliasing = rtt::TEXTURE_ANTI_ALIASING::SSAA_4;
		else if(anti_aliasing_str == "4/3xSSAA+FXAA") config.anti_aliasing = rtt::TEXTURE_ANTI_ALIASING::SSAA_4_3_FXAA;
		else if(anti_aliasing_str == "2xSSAA+FXAA") config.anti_aliasing = rtt::TEXTURE_ANTI_ALIASING::SSAA_2_FXAA;
		else config.anti_aliasing = rtt::TEXTURE_ANTI_ALIASING::NONE;
		
		config.disabled_extensions = config_doc.get<string>("config.graphic_device.disabled_extensions", "");
		config.force_device = config_doc.get<string>("config.graphic_device.force_device", "");
		config.force_vendor = config_doc.get<string>("config.graphic_device.force_vendor", "");
		
		config.upscaling = config_doc.get<float>("config.inferred.upscaling", 1.0f);
		config.geometry_light_scaling = config_doc.get<float>("config.inferred.geometry_light_scaling", 1.0f);
		config.geometry_light_scaling = core::clamp(config.geometry_light_scaling, 0.5f, 1.0f);
	}
	
	if(console_only_) {
		mode = INIT_MODE::CONSOLE;
		// create extension class object
		exts = new ext(mode, &config.disabled_extensions, &config.force_device, &config.force_vendor);
		log_debug("initializing albion 2 engine in console only mode");
	}
	else {
		mode = INIT_MODE::GRAPHICAL;
		log_debug("initializing albion 2 engine in console + graphical mode");
		
		// load icon
		if(ico_ != nullptr) load_ico(ico_);
		
		floor::acquire_context();
		
		// make an early clear
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		swap();
		evt->handle_events(); // this will effectively create/open the window on some platforms
		
		// create extension class object
		exts = new ext(engine::mode, &config.disabled_extensions, &config.force_device, &config.force_vendor);
		
		// capability test
#if !defined(A2E_IOS)
		if(!exts->is_gl_version(3, 2)) { // TODO: check for shader support! (use recognized gl version)
			log_error("A2E doesn't support your graphic device! OpenGL 3.2 is the minimum requirement.");
			SDL_Delay(10000);
			exit(1);
		}
#else
		if(!exts->is_gl_version(2, 0)) {
			log_error("A2E doesn't support your graphic device! OpenGL ES 2.0 is the minimum requirement.");
			SDL_Delay(10000);
			exit(1);
		}
#endif
		
		//
#if defined(A2E_DEBUG)
		gl_timer::init();
#endif
		
		int tmp = 0;
		SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &tmp);
		log_debug("double buffering %s", tmp == 1 ? "enabled" : "disabled");
		
		// print out some opengl informations
		log_debug("vendor: %s", glGetString(GL_VENDOR));
		log_debug("renderer: %s", glGetString(GL_RENDERER));
		log_debug("version: %s", glGetString(GL_VERSION));
		
		// no support for ms gdi driver ...
		if(strcmp((const char*)glGetString(GL_RENDERER), "GDI Generic") == 0) {
			log_error("A2E doesn't support the MS GDI Generic driver!\nGo and install one of these (that match your graphics card):\nhttp://www.ati.com  http://www.nvidia.com  http://www.intel.com");
			SDL_Delay(10000);
			exit(1);
		}
		
		if(SDL_GetCurrentVideoDriver() == nullptr) {
			log_error("couldn't get video driver: %s!", SDL_GetError());
		}
		else log_debug("video driver: %s", SDL_GetCurrentVideoDriver());
		
		evt->set_ldouble_click_time((unsigned int)config.ldouble_click_time);
		evt->set_rdouble_click_time((unsigned int)config.rdouble_click_time);
		evt->set_mdouble_click_time((unsigned int)config.mdouble_click_time);
		
		// initialize ogl
		init_gl();
		log_debug("opengl initialized");
		
		// resize stuff
		resize_window();
		
		// set and check which anti-aliasing modes are supported (ranging from worst to best)
		supported_aa_modes.push_back(rtt::TEXTURE_ANTI_ALIASING::NONE);
		supported_aa_modes.push_back(rtt::TEXTURE_ANTI_ALIASING::FXAA);
		supported_aa_modes.push_back(rtt::TEXTURE_ANTI_ALIASING::SSAA_2);
		supported_aa_modes.push_back(rtt::TEXTURE_ANTI_ALIASING::SSAA_4);
		supported_aa_modes.push_back(rtt::TEXTURE_ANTI_ALIASING::SSAA_4_3_FXAA);
		supported_aa_modes.push_back(rtt::TEXTURE_ANTI_ALIASING::SSAA_2_FXAA);
		set_anti_aliasing(config.anti_aliasing);
		
		// set and check anisotropic
		set_anisotropic(config.anisotropic);
		
		// create texture manager and render to texture object
		t = new texman(exts, config.anisotropic);
		r = new rtt(this, exts);
		
		// set standard texture filtering + anisotropic filtering
		t->set_filtering(config.filtering);
		
		// get standard (sdl internal) cursor and create cursor data
		standard_cursor = SDL_GetCursor();
		cursors["STANDARD"] = standard_cursor;
		
		// init/create shaders, init gfx
		shd = new shader(this);
		gfx2d::init(this);
		
		// draw the loading screen/image
		start_draw();
		start_2d_draw();
		a2e_texture load_tex = t->add_texture(floor::data_path("loading.png"), TEXTURE_FILTERING::LINEAR, 0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		const uint2 load_tex_draw_size(load_tex->width/2, load_tex->height/2);
		const uint2 img_offset((unsigned int)floor::get_width()/2 - load_tex_draw_size.x/2,
							   (unsigned int)floor::get_height()/2 - load_tex_draw_size.y/2);
		gfx2d::set_blend_mode(gfx2d::BLEND_MODE::PRE_MUL);
		gfx2d::draw_rectangle_texture(rect(img_offset.x, img_offset.y,
										   img_offset.x + load_tex_draw_size.x,
										   img_offset.y + load_tex_draw_size.y),
									  load_tex->tex(),
									  coord(0.0f, 1.0f), coord(1.0f, 0.0f));
		stop_2d_draw();
		stop_draw();
		
		// create scene
		sce = new scene(this);
		
		// create gui
		if(config.ui_anti_aliasing > 2) config.ui_anti_aliasing = core::next_pot((unsigned int)config.ui_anti_aliasing);
		if(exts->get_max_samples() < config.ui_anti_aliasing) {
			config.ui_anti_aliasing = exts->get_max_samples();
			log_error("your chosen gui anti-aliasing mode isn't supported by your graphic card - using \"%u\" instead!",
					  config.ui_anti_aliasing);
		}
		else log_debug("using \"%ux\" gui anti-aliasing", config.ui_anti_aliasing);
		
		switch(config.ui_anti_aliasing) {
			case 0: config.ui_anti_aliasing_enum = rtt::TEXTURE_ANTI_ALIASING::NONE; break;
			case 2: config.ui_anti_aliasing_enum = rtt::TEXTURE_ANTI_ALIASING::MSAA_2; break;
			case 4: config.ui_anti_aliasing_enum = rtt::TEXTURE_ANTI_ALIASING::MSAA_4; break;
			case 8: config.ui_anti_aliasing_enum = rtt::TEXTURE_ANTI_ALIASING::MSAA_8; break;
			case 16: config.ui_anti_aliasing_enum = rtt::TEXTURE_ANTI_ALIASING::MSAA_16; break;
			case 32: config.ui_anti_aliasing_enum = rtt::TEXTURE_ANTI_ALIASING::MSAA_32; break;
			case 64: config.ui_anti_aliasing_enum = rtt::TEXTURE_ANTI_ALIASING::MSAA_64; break;
			default: break;
		}
		ui = new gui(this, "standard"); // TODO: add config setting for theme name
		
		floor::release_context();
	}
}

void engine::destroy() {
	log_debug("deleting engine object");
	
	floor::acquire_context();
	
	for(const auto& cursor : cursors) {
		if(cursor.first != "STANDARD") {
			SDL_FreeCursor(cursor.second);
		}
	}
	cursors.clear();
	
	evt->remove_event_handler(*window_handler);
	delete window_handler;
	
	gfx2d::destroy();
	
	if(t != nullptr) delete t;
	if(exts != nullptr) delete exts;
	if(shd != nullptr) delete shd;
	if(ui != nullptr) delete ui;
	if(sce != nullptr) delete sce;
	if(ocl != nullptr) {
		delete ocl;
		ocl = nullptr;
	}
	
#if defined(A2E_DEBUG)
	gl_timer::destroy();
#endif
	floor::release_context();
	
	log_debug("engine object deleted");
	floor::destroy();
}

/*! starts drawing the window
 */
void engine::start_draw() {
	floor::acquire_context();
	gl_timer::stop_frame();
	gl_timer::state_check();
	gl_timer::start_frame();
	
	// draws ogl stuff
	glBindFramebuffer(GL_FRAMEBUFFER, A2E_DEFAULT_FRAMEBUFFER);
	glViewport(0, 0, (unsigned int)floor::get_width(), (unsigned int)floor::get_height());
	
	// clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// reset model view matrix
	modelview_matrix.identity();
	translation_matrix.identity();
	rotation_matrix.identity();
	mvp_matrix = projection_matrix;
	
#if !defined(A2E_IOS)
	glBindVertexArray(global_vao);
#endif
}

/*! stops drawing the window
 */
void engine::stop_draw() {
	// draw scene and gui
	if(sce != nullptr) sce->draw();
	if(ui != nullptr) ui->draw();
	
	//
	swap();
	
	GLenum error = glGetError();
	switch(error) {
		case GL_NO_ERROR:
			break;
		case GL_INVALID_ENUM:
			log_error("OpenGL error: invalid enum!");
			break;
		case GL_INVALID_VALUE:
			log_error("OpenGL error: invalid value!");
			break;
		case GL_INVALID_OPERATION:
			log_error("OpenGL error: invalid operation!");
			break;
		case GL_OUT_OF_MEMORY:
			log_error("OpenGL error: out of memory!");
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			log_error("OpenGL error: invalid framebuffer operation!");
			break;
		default:
			log_error("unknown OpenGL error: %u!");
			break;
	}

	// fps "limiter"
	if(config.fps_limit != 0) SDL_Delay((unsigned int)config.fps_limit);
	
	// check for shader/kernel reload (this is safe to do here)
	if(reload_shaders_flag) {
		reload_shaders_flag = false;
		glFlush();
		glFinish();
		shd->reload_shaders();
	}
	
	gl_timer::mark("FRAME_END");
	floor::release_context();
}

void engine::push_ogl_state() {
	// make a full soft-context-switch
#if !defined(A2E_IOS)
	glGetIntegerv(GL_BLEND_SRC, &pushed_blend_src);
	glGetIntegerv(GL_BLEND_DST, &pushed_blend_dst);
#endif
	glGetIntegerv(GL_BLEND_SRC_RGB, &pushed_blend_src_rgb);
	glGetIntegerv(GL_BLEND_DST_RGB, &pushed_blend_dst_rgb);
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &pushed_blend_src_alpha);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &pushed_blend_dst_alpha);
}

void engine::pop_ogl_state() {
	// make a full soft-context-switch, restore all values
	
	glBlendFunc(pushed_blend_src, pushed_blend_dst);
	glBlendFuncSeparate(pushed_blend_src_rgb, pushed_blend_dst_rgb,
						pushed_blend_src_alpha, pushed_blend_dst_alpha);
}

/*! opengl initialization function
 */
void engine::init_gl() {
	// set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	// depth buffer setup
	glClearDepth(1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// less/equal depth test
	glDepthFunc(GL_LESS);
	// enable backface culling
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glDisable(GL_STENCIL_TEST);
	
#if !defined(A2E_IOS)
	//
	static bool vao_init = false;
	if(!vao_init) {
		vao_init = true;
		glGenVertexArrays(1, &global_vao);
	}
	glBindVertexArray(global_vao);
#endif
}

/* function to reset our viewport after a window resize
 */
void engine::resize_window() {
	// set the viewport
	glViewport(0, 0, (GLsizei)floor::get_width(), (GLsizei)floor::get_height());

	// projection matrix
	// set perspective with fov (default = 72) and near/far plane value (default = 1.0f/1000.0f)
	projection_matrix.perspective(floor::get_fov(), float(floor::get_width()) / float(floor::get_height()),
								  floor::get_near_far_plane().x, floor::get_near_far_plane().y);

	// model view matrix
	modelview_matrix.identity();
	translation_matrix.identity();
	rotation_matrix.identity();
	mvp_matrix = projection_matrix;
}

/*! sets the position of the user/viewer
 *  @param xpos x coordinate
 *  @param ypos y coordinate
 *  @param zpos z coordinate
 */
void engine::set_position(float xpos, float ypos, float zpos) {
	position.set(xpos, ypos, zpos);
	
	translation_matrix = matrix4f().translate(xpos, ypos, zpos);
	modelview_matrix = translation_matrix * rotation_matrix;
	mvp_matrix = modelview_matrix * projection_matrix;
}

/*! sets the rotation of the user/viewer
 *  @param xrot x rotation
 *  @param yrot y rotation
 */
void engine::set_rotation(float xrot, float yrot) {
	rotation.x = xrot;
	rotation.y = yrot;
	
	rotation_matrix = matrix4f().rotate_y(yrot) * matrix4f().rotate_x(xrot);
	modelview_matrix = translation_matrix * rotation_matrix;
	mvp_matrix = modelview_matrix * projection_matrix;
}

/*! returns the position of the user
 */
float3* engine::get_position() {
	return &position;
}

/*! returns the rotation of the user
 */
float3* engine::get_rotation() {
	return &rotation;
}

/*! starts drawing the 2d elements and initializes the opengl functions for that
 */
void engine::start_2d_draw() {
	start_2d_draw((unsigned int)floor::get_width(), (unsigned int)floor::get_height());
}

void engine::start_2d_draw(const unsigned int width, const unsigned int height) {
	glViewport(0, 0, width, height);

	// we need an orthogonal view (2d) for drawing 2d elements
	push_projection_matrix();
	projection_matrix.ortho(0, width, 0, height, -1.0, 1.0);

	push_modelview_matrix();
	modelview_matrix.identity();
	
	glFrontFace(GL_CW);
	mvp_matrix = projection_matrix;
	glDisable(GL_CULL_FACE); // TODO: GL3, remove again
	
	// shaders are using pre-multiplied alpha
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

/*! stops drawing the 2d elements
 */
void engine::stop_2d_draw() {
	pop_projection_matrix();
	pop_modelview_matrix();
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE); // TODO: GL3, remove again
}

/*! returns the type of the initialization (0 = GRAPHICAL, 1 = CONSOLE)
 */
const INIT_MODE& engine::get_init_mode() {
	return engine::mode;
}

/*! returns the texman class
 */
texman* engine::get_texman() {
	return engine::t;
}

/*! returns the extensions class
 */
ext* engine::get_ext() {
	return engine::exts;
}

/*! returns the rtt class
 */
rtt* engine::get_rtt() {
	return engine::r;
}

/*! returns the shader class
 */
shader* engine::get_shader() {
	return engine::shd;
}

/*! returns the gui class
 */
gui* engine::get_gui() {
	return engine::ui;
}

/*! returns a pointer to the scene class
 */
scene* engine::get_scene() {
	return engine::sce;
}

/*! returns data path + shader path + str
 *  @param str str we want to "add" to the data + shader path
 */
string engine::shader_path(const string& str) const {
	return floor::data_path(shaderpath + str);
}

void engine::load_ico(const char* ico) {
	SDL_SetWindowIcon(floor::get_window(), IMG_Load(floor::data_path(ico).c_str()));
}

TEXTURE_FILTERING engine::get_filtering() const {
	return config.filtering;
}

void engine::set_filtering(const TEXTURE_FILTERING& filtering) {
	if(filtering == config.filtering) return;
	config.filtering = filtering;
	t->set_filtering(filtering);
}

size_t engine::get_anisotropic() const {
	return config.anisotropic;
}

void engine::set_anisotropic(const size_t& anisotropic) {
	config.anisotropic = anisotropic;
	if(config.anisotropic > exts->get_max_anisotropic_filtering()) {
		config.anisotropic = exts->get_max_anisotropic_filtering();
		log_error("your chosen anisotropic-filtering value isn't supported by your graphic card - using \"%u\" instead!", config.anisotropic);
	}
	else log_debug("using \"%ux\" anisotropic-filtering", config.anisotropic);
}

rtt::TEXTURE_ANTI_ALIASING engine::get_anti_aliasing() const {
	return config.anti_aliasing;
}

void engine::set_anti_aliasing(const rtt::TEXTURE_ANTI_ALIASING& anti_aliasing) {
	//
	const bool recreate_buffers = (anti_aliasing != config.anti_aliasing);
	
	//
	bool chosen_aa_mode_supported = false;
	config.anti_aliasing = anti_aliasing;
	for(const auto& aa_mode : supported_aa_modes) {
		if(aa_mode == config.anti_aliasing) chosen_aa_mode_supported = true;
	}
	
	// if the chosen anti-aliasing mode isn't supported, use the next best one
	if(!chosen_aa_mode_supported) {
		config.anti_aliasing = supported_aa_modes.back();
		log_error("your chosen anti-aliasing mode isn't supported by your graphic card - using \"%s\" instead!", rtt::TEXTURE_ANTI_ALIASING_STR[(unsigned int)config.anti_aliasing]);
	}
	else {
		log_debug("using \"%s\" anti-aliasing",
				  rtt::TEXTURE_ANTI_ALIASING_STR[(unsigned int)config.anti_aliasing]);
	}
	
	if(recreate_buffers) {
		evt->add_event(EVENT_TYPE::WINDOW_RESIZE,
					   make_shared<window_resize_event>(SDL_GetTicks(),
														size2(floor::get_width(), floor::get_height())));
	}
}

matrix4f* engine::get_projection_matrix() {
	return &(engine::projection_matrix);
}

matrix4f* engine::get_modelview_matrix() {
	return &(engine::modelview_matrix);
}

matrix4f* engine::get_mvp_matrix() {
	return &(engine::mvp_matrix);
}

matrix4f* engine::get_translation_matrix() {
	return &(engine::translation_matrix);
}

matrix4f* engine::get_rotation_matrix() {
	return &(engine::rotation_matrix);
}

SDL_Cursor* engine::add_cursor(const char* name, const char** raw_data, unsigned int xsize, unsigned int ysize, unsigned int hotx, unsigned int hoty) {
	if(cursors.count(name)) {
		log_error("cursor with such a name (%s) already exists!", name);
		return nullptr;
	}

	if((xsize != 16 || ysize != 16) && (xsize != 32 || ysize != 32)) {
		log_error("invalid cursor size (%ux%u) - must be either 16x16 or 32x32!", xsize, ysize);
		return nullptr;
	}

	if(xsize == 16) {
		cursor_data = cursor_data16;
		cursor_mask = cursor_mask16;
		memset(cursor_data, 0, 2*16);
		memset(cursor_mask, 0, 2*16);
	}
	else {
		cursor_data = cursor_data32;
		cursor_mask = cursor_mask32;
		memset(cursor_data, 0, 4*32);
		memset(cursor_mask, 0, 4*32);
	}

	unsigned int data_byte = 0;
	unsigned int data_bit = 0;
	for(unsigned int cy = 0; cy < xsize; cy++) {
		for(unsigned int cx = 0; cx < ysize; cx++) {
			data_byte = cy*(xsize/8) + cx/8;
			data_bit = 7 - (cx - (cx/8)*8);
			switch(raw_data[cy][cx]) {
				case ' ':
					// nothing ...
					break;
				case 'X':
					cursor_data[data_byte] |= (1 << data_bit);
					cursor_mask[data_byte] |= (1 << data_bit);
					break;
				case '.':
					cursor_mask[data_byte] |= (1 << data_bit);
					break;
				case 'I':
					cursor_data[data_byte] |= (1 << data_bit);
					break;
				default:
					break;
			}
		}
	}
	cursors[name] = SDL_CreateCursor(cursor_data, cursor_mask, (int)xsize, (int)ysize, (int)hotx, (int)hoty);
	return cursors[name];
}

void engine::set_cursor(SDL_Cursor* cursor) {
	SDL_SetCursor(cursor);
}

SDL_Cursor* engine::get_cursor(const char* name) {
	if(!cursors.count(name)) {
		log_error("no cursor with such a name (%s) exists!", name);
		return nullptr;
	}
	return cursors[name];
}

unsigned int engine::get_key_repeat() const {
	return (unsigned int)config.key_repeat;
}

unsigned int engine::get_ldouble_click_time() const {
	return (unsigned int)config.ldouble_click_time;
}

unsigned int engine::get_mdouble_click_time() const {
	return (unsigned int)config.mdouble_click_time;
}

unsigned int engine::get_rdouble_click_time() const {
	return (unsigned int)config.rdouble_click_time;
}

const string& engine::get_disabled_extensions() const {
	return config.disabled_extensions;
}

const string& engine::get_force_device() const {
	return config.force_device;
}

const string& engine::get_force_vendor() const {
	return config.force_vendor;
}

float engine::get_upscaling() const {
	return config.upscaling;
}

void engine::set_upscaling(const float& factor) {
	if(factor == config.upscaling) return;
	config.upscaling = factor;
	evt->add_event(EVENT_TYPE::WINDOW_RESIZE,
				   make_shared<window_resize_event>(SDL_GetTicks(),
													size2(floor::get_width(), floor::get_height())));
}

float engine::get_geometry_light_scaling() const {
	return config.geometry_light_scaling;
}

void engine::set_geometry_light_scaling(const float& factor) {
	if(factor == config.geometry_light_scaling) return;
	config.geometry_light_scaling = factor;
	evt->add_event(EVENT_TYPE::WINDOW_RESIZE,
				   make_shared<window_resize_event>(SDL_GetTicks(),
													size2(floor::get_width(), floor::get_height())));
}

const string engine::get_version() const {
	return A2E_VERSION_STRING;
}

void engine::swap() {
	gl_timer::mark("SWAP_START");
	floor::swap();
	gl_timer::mark("SWAP_END");
}

void engine::push_projection_matrix() {
	projm_stack.push_back(new matrix4f(projection_matrix));
}

void engine::pop_projection_matrix() {
	matrix4f* pmat = projm_stack.back();
	projection_matrix = *pmat;
	delete pmat;
	projm_stack.pop_back();
}

void engine::push_modelview_matrix() {
	mvm_stack.push_back(new matrix4f(modelview_matrix));
}

void engine::pop_modelview_matrix() {
	matrix4f* pmat = mvm_stack.back();
	modelview_matrix = *pmat;
	mvp_matrix = modelview_matrix * projection_matrix;
	delete pmat;
	mvm_stack.pop_back();
}

void engine::reload_shaders() {
	reload_shaders_flag = true;
}

const rtt::TEXTURE_ANTI_ALIASING& engine::get_ui_anti_aliasing() const {
	return config.ui_anti_aliasing_enum;
}

bool engine::window_event_handler(EVENT_TYPE type floor_unused, shared_ptr<event_object> obj floor_unused) {
	return true;
}

// TODO: better location!
// from global.hpp:
DRAW_MODE operator|(const DRAW_MODE& e0, const DRAW_MODE& e1) {
	return (DRAW_MODE)((typename underlying_type<DRAW_MODE>::type)e0 |
					   (typename underlying_type<DRAW_MODE>::type)e1);
}
DRAW_MODE& operator|=(DRAW_MODE& e0, const DRAW_MODE& e1) {
	e0 = e0 | e1;
	return e0;
}

DRAW_MODE operator&(const DRAW_MODE& e0, const DRAW_MODE& e1) {
	return (DRAW_MODE)((typename underlying_type<DRAW_MODE>::type)e0 &
					   (typename underlying_type<DRAW_MODE>::type)e1);
}
DRAW_MODE& operator&=(DRAW_MODE& e0, const DRAW_MODE& e1) {
	e0 = e0 & e1;
	return e0;
}
