/*  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2012 Florian Ziesche
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

#include "engine.h"
#include "a2e_version.h"
#include "rendering/shader.h"
#include "cl/opencl.h"
#include "gui/gui.h"
#include "rendering/gfx2d.h"
#include "scene/scene.h"

// dll main for windows dll export
#ifdef __WINDOWS__
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
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

/*! this is used to set an absolute data path depending on call path (path from where the binary is called/started),
 *! which is mostly needed when the binary is opened via finder under os x or any file manager under linux
 */
engine::engine(const char* callpath_, const char* datapath_) {
	logger::init();
	
	engine::callpath = callpath_;
	engine::datapath = callpath_;
	engine::rel_datapath = datapath_;

#ifndef __WINDOWS__
	const char dir_slash = '/';
#else
	const char dir_slash = '\\';
#endif
	
#if defined(A2E_IOS)
	// strip one "../"
	const size_t cdup_pos = rel_datapath.find("../");
	if(cdup_pos != string::npos) {
		rel_datapath = (rel_datapath.substr(0, cdup_pos) +
						rel_datapath.substr(cdup_pos+3, rel_datapath.length()-cdup_pos-3));
	}
#endif
	
	engine::datapath = datapath.substr(0, datapath.rfind(dir_slash)+1) + rel_datapath;

#ifdef CYGWIN
	engine::callpath = "./";
	engine::datapath = callpath_;
	engine::datapath = datapath.substr(0, datapath.rfind("/")+1) + rel_datapath;
#endif
	
	create();
}

/*! there is no function currently
 */
engine::~engine() {
	a2e_debug("deleting engine object");

	for(const auto& cursor : cursors) {
		if(cursor.first != "STANDARD") {
			SDL_FreeCursor(cursor.second);
		}
	}
	cursors.clear();
	
	e->remove_event_handler(*window_handler);
	delete window_handler;
	
	gfx2d::destroy();

	if(c != nullptr) delete c;
	if(f != nullptr) delete f;
	if(t != nullptr) delete t;
	if(exts != nullptr) delete exts;
	if(x != nullptr) delete x;
	if(u != nullptr) delete u;
	if(ocl != nullptr) delete ocl;
	if(shd != nullptr) delete shd;
	if(ui != nullptr) delete ui;
	if(sce != nullptr) delete sce;
	
	// delete this at the end, b/c other classes will remove event handlers
	if(e != nullptr) delete e;

	a2e_debug("engine object deleted");
	
	SDL_GL_DeleteContext(config.ctx);
	SDL_DestroyWindow(config.wnd);
	SDL_Quit();
	
	logger::destroy();
}

void engine::create() {
#if !defined(__WINDOWS__) && !defined(CYGWIN)
	if(datapath.size() > 0 && datapath[0] == '.') {
		// strip leading '.' from datapath if there is one
		datapath.erase(0, 1);
		
		char working_dir[8192];
		memset(working_dir, 0, 8192);
		getcwd(working_dir, 8192);
		
		datapath = working_dir + datapath;
	}
#elif defined(CYGWIN)
	// do nothing
#else
	char working_dir[8192];
	memset(working_dir, 0, 8192);
	getcwd(working_dir, 8192);

	size_t strip_pos = datapath.find("\\.\\");
	if(strip_pos != string::npos) {
		datapath.erase(strip_pos, 3);
	}
	
	bool add_bin_path = (working_dir == datapath.substr(0, datapath.length()-1)) ? false : true;
	if(!add_bin_path) datapath = working_dir + string("\\") + (add_bin_path ? datapath : "");
	else {
		if(datapath[datapath.length()-1] == '/') {
			datapath = datapath.substr(0, datapath.length()-1);
		}
		datapath += string("\\");
	}

#endif
	
#ifdef __APPLE__
	// check if datapath contains a 'MacOS' string (indicates that the binary is called from within an OS X .app or via complete path from the shell)
	if(datapath.find("MacOS") != string::npos) {
		// if so, add "../../../" to the datapath, since we have to relocate the datapath if the binary is inside an .app
		datapath.insert(datapath.find("MacOS")+6, "../../../");
	}
#endif
	
	// condense datapath
	datapath = core::strip_path(datapath);
	
	shaderpath = "shader/";
	kernelpath = "kernels/";
	cursor_visible = true;
	
	fps = 0;
	fps_counter = 0;
	fps_time = 0;
	frame_time = 0.0f;
	frame_time_sum = 0;
	frame_time_counter = 0;
	new_fps_count = false;
	standard_cursor = nullptr;
	cursor_data = nullptr;
	cursor_mask = nullptr;
	global_vao = 0;
	
	u = new unicode();
	f = new file_io();
	c = new core();
	x = new xml(this);
	e = new event();
	
	window_handler = new event::handler(this, &engine::window_event_handler);
	e->add_internal_event_handler(*window_handler, EVENT_TYPE::WINDOW_RESIZE);
	
	AtomicSet(&reload_shaders_flag, 0);
	AtomicSet(&reload_kernels_flag, 0);
	
	// print out engine info
	a2e_debug("%s", (A2E_VERSION_STRING).c_str());
	
	// load config
	config_doc = x->process_file(data_path("config.xml"));
	if(config_doc.valid) {
		config.width = config_doc.get<size_t>("config.screen.width", 640);
		config.height = config_doc.get<size_t>("config.screen.height", 480);
		config.fullscreen = config_doc.get<bool>("config.screen.fullscreen", false);
		config.vsync = config_doc.get<bool>("config.screen.vsync", false);
		config.stereo = config_doc.get<bool>("config.screen.stereo", false);
		
		config.fov = config_doc.get<float>("config.projection.fov", 72.0f);
		config.near_far_plane.x = config_doc.get<float>("config.projection.near", 1.0f);
		config.near_far_plane.y = config_doc.get<float>("config.projection.far", 1000.0f);
		
		config.key_repeat = config_doc.get<size_t>("config.input.key_repeat", 200);
		config.ldouble_click_time = config_doc.get<size_t>("config.input.ldouble_click_time", 200);
		config.mdouble_click_time = config_doc.get<size_t>("config.input.mdouble_click_time", 200);
		config.rdouble_click_time = config_doc.get<size_t>("config.input.rdouble_click_time", 200);
		
		config.fps_limit = config_doc.get<size_t>("config.sleep.time", 0);
		
		config.server.port = (unsigned short int)config_doc.get<size_t>("config.server.port", 0);
		config.server.max_clients = (unsigned int)config_doc.get<size_t>("config.server.max_clients", 0);
		
		config.client.client_name = config_doc.get<string>("config.client.name", "");
		config.client.server_name = config_doc.get<string>("config.client.server", "");
		config.client.port = (unsigned short int)config_doc.get<size_t>("config.client.port", 0);
		config.client.lis_port = (unsigned short int)config_doc.get<size_t>("config.client.lis_port", 0);
		
		string filtering_str = config_doc.get<string>("config.graphic.filtering", "");
		if(filtering_str == "POINT") config.filtering = texture_object::TF_POINT;
		else if(filtering_str == "LINEAR") config.filtering = texture_object::TF_LINEAR;
		else if(filtering_str == "BILINEAR") config.filtering = texture_object::TF_BILINEAR;
		else if(filtering_str == "TRILINEAR") config.filtering = texture_object::TF_TRILINEAR;
		else config.filtering = texture_object::TF_POINT;
		
		config.anisotropic = config_doc.get<size_t>("config.graphic.anisotropic", 0);
		
		string anti_aliasing_str = config_doc.get<string>("config.graphic.anti_aliasing", "");
		if(anti_aliasing_str == "NONE") config.anti_aliasing = rtt::TAA_NONE;
		else if(anti_aliasing_str == "FXAA") config.anti_aliasing = rtt::TAA_FXAA;
		else if(anti_aliasing_str == "2xSSAA") config.anti_aliasing = rtt::TAA_SSAA_2;
		//else if(anti_aliasing_str == "4xSSAA") config.anti_aliasing = rtt::TAA_SSAA_4;
		else if(anti_aliasing_str == "4/3xSSAA+FXAA") config.anti_aliasing = rtt::TAA_SSAA_4_3_FXAA;
		else if(anti_aliasing_str == "2xSSAA+FXAA") config.anti_aliasing = rtt::TAA_SSAA_2_FXAA;
		else config.anti_aliasing = rtt::TAA_NONE;
		
		config.disabled_extensions = config_doc.get<string>("config.graphic_device.disabled_extensions", "");
		config.force_device = config_doc.get<string>("config.graphic_device.force_device", "");
		config.force_vendor = config_doc.get<string>("config.graphic_device.force_vendor", "");
		
		config.inferred_scale = config_doc.get<size_t>("config.inferred.scale", 4);
		
		config.opencl_platform = config_doc.get<size_t>("config.opencl.platform", 0);
		config.clear_cache = config_doc.get<bool>("config.opencl.clear_cache", false);
	}
}

/*! initializes the engine in console + graphical or console only mode
 *  @param console the initialization mode (false = gfx/console, true = console only)
 *  @param width the window width
 *  @param height the window height
 *  @param depth the color depth of the window (16, 24 or 32)
 *  @param fullscreen bool if the window is drawn in fullscreen mode
 */
void engine::init(bool console, unsigned int width, unsigned int height,
				  bool fullscreen, bool vsync, const char* ico) {
	if(console == true) {
		engine::mode = engine::CONSOLE;
		// create extension class object
		exts = new ext(engine::mode, &config.disabled_extensions, &config.force_device, &config.force_vendor);
		a2e_debug("initializing albion 2 engine in console only mode");
	}
	else {
		config.width = width;
		config.height = height;
		config.fullscreen = fullscreen;
		config.vsync = vsync;

		engine::init(ico);
	}
}

/*! initializes the engine in console + graphical mode
 */
void engine::init(const char* ico) {
	engine::mode = engine::GRAPHICAL;
	a2e_debug("initializing albion 2 engine in console + graphical mode");

	// initialize sdl
	if(SDL_Init(SDL_INIT_VIDEO) == -1) {
		a2e_error("can't init SDL: %s", SDL_GetError());
		exit(1);
	}
	else {
		a2e_debug("sdl initialized");
	}
	atexit(SDL_Quit);

	// set some flags
	config.flags |= SDL_WINDOW_OPENGL;
	config.flags |= SDL_WINDOW_SHOWN;
	
#if !defined(A2E_IOS)
	config.flags |= SDL_WINDOW_INPUT_FOCUS;
	config.flags |= SDL_WINDOW_MOUSE_FOCUS;

	int2 windows_pos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	if(config.fullscreen) {
		config.flags |= SDL_WINDOW_FULLSCREEN;
		config.flags |= SDL_WINDOW_BORDERLESS;
		windows_pos.set(0, 0);
		a2e_debug("fullscreen enabled");
	}
	else {
		a2e_debug("fullscreen disabled");
		config.flags |= SDL_WINDOW_RESIZABLE;
	}
#else
	config.flags |= SDL_WINDOW_FULLSCREEN;
	config.flags |= SDL_WINDOW_RESIZABLE;
	config.flags |= SDL_WINDOW_BORDERLESS;
#endif

	a2e_debug("vsync %s", config.vsync ? "enabled" : "disabled");
	
	// gl attributes
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
#if !defined(A2E_IOS)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	
	//
	SDL_DisplayMode fullscreen_mode;
	SDL_zero(fullscreen_mode);
	fullscreen_mode.format = SDL_PIXELFORMAT_RGBA8888;
	fullscreen_mode.w = config.width;
	fullscreen_mode.h = config.height;
#endif

	// create screen
#if !defined(A2E_IOS)
	config.wnd = SDL_CreateWindow("A2E", windows_pos.x, windows_pos.y, (unsigned int)config.width, (unsigned int)config.height, config.flags);
#else
	config.wnd = SDL_CreateWindow("A2E", 0, 0, (unsigned int)config.width, (unsigned int)config.height, config.flags);
#endif
	if(config.wnd == nullptr) {
		a2e_error("can't create window: %s", SDL_GetError());
		exit(1);
	}
	else {
		SDL_GetWindowSize(config.wnd, (int*)&config.width, (int*)&config.height);
		a2e_debug("video mode set: w%u h%u", config.width, config.height);
	}
	
#if defined(A2E_IOS)
	if(SDL_SetWindowDisplayMode(config.wnd, &fullscreen_mode) < 0) {
		a2e_error("can't set up fullscreen display mode: %s", SDL_GetError());
		exit(1);
	}
	SDL_GetWindowSize(config.wnd, (int*)&config.width, (int*)&config.height);
	a2e_debug("fullscreen mode set: w%u h%u", config.width, config.height);
	SDL_ShowWindow(config.wnd);
#endif
	
	// load icon
	if(ico != nullptr) load_ico(ico);
	
	config.ctx = SDL_GL_CreateContext(config.wnd);
	if(config.ctx == nullptr) {
		a2e_error("can't create opengl context: %s", SDL_GetError());
		exit(1);
	}
#if !defined(A2E_IOS)
	SDL_GL_SetSwapInterval(config.vsync ? 1 : 0); // has to be set after context creation
#endif
	acquire_gl_context();
	
	// TODO: this is only a rudimentary solution, think of or wait for a better one ...
#if !defined(A2E_NO_OPENCL)
	ocl = new opencl(core::strip_path(string(datapath + kernelpath)).c_str(), f, config.wnd, config.clear_cache); // use absolute path
#endif
	
	// enable multi-threaded opengl context when on os x
#if defined(__APPLE__) && 0
	CGLError cgl_err = CGLEnable(CGLGetCurrentContext(), kCGLCEMPEngine);
	if(cgl_err != kCGLNoError) {
		a2e_error("unable to set multi-threaded opengl context (%X)!", cgl_err);
	}
	else {
		a2e_debug("multi-threaded opengl context enabled!");
	}
#endif
	
	// make an early clear
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	swap();
	release_gl_context(); // release, before we call handle_events, since this might acquire the context!
	e->handle_events(); // this will effectively create/open the window on some platforms
	acquire_gl_context();

	// create extension class object
	exts = new ext(engine::mode, &config.disabled_extensions, &config.force_device, &config.force_vendor);
	
	// capability test
#if !defined(A2E_IOS)
	if(!exts->is_gl_version(3, 2)) { // TODO: check for shader support! (use recognized gl version)
		a2e_error("A2E doesn't support your graphic device! OpenGL 3.2 is the minimum requirement.");
		SDL_Delay(10000);
		exit(1);
	}
#else
	if(!exts->is_gl_version(2, 0)) {
		a2e_error("A2E doesn't support your graphic device! OpenGL ES 2.0 is the minimum requirement.");
		SDL_Delay(10000);
		exit(1);
	}
#endif
	
	int tmp = 0;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &tmp);
	a2e_debug("double buffering %s", tmp == 1 ? "enabled" : "disabled");

	// print out some opengl informations
	a2e_debug("vendor: %s", glGetString(GL_VENDOR));
	a2e_debug("renderer: %s", glGetString(GL_RENDERER));
	a2e_debug("version: %s", glGetString(GL_VERSION));
	
	if(SDL_GetCurrentVideoDriver() == nullptr) {
		a2e_error("couldn't get video driver: %s!", SDL_GetError());
	}
	else a2e_debug("video driver: %s", SDL_GetCurrentVideoDriver());
	
	e->set_ldouble_click_time((unsigned int)config.ldouble_click_time);
	e->set_rdouble_click_time((unsigned int)config.rdouble_click_time);
	e->set_mdouble_click_time((unsigned int)config.mdouble_click_time);
	
	// initialize ogl
	init_gl();
	a2e_debug("opengl initialized");

	// resize stuff
	resize_window();

	// check which anti-aliasing modes are supported (ranging from worst to best)
	supported_aa_modes.push_back(rtt::TAA_NONE);
	supported_aa_modes.push_back(rtt::TAA_FXAA);
	supported_aa_modes.push_back(rtt::TAA_SSAA_2);
	supported_aa_modes.push_back(rtt::TAA_SSAA_4);
	supported_aa_modes.push_back(rtt::TAA_SSAA_4_3_FXAA);
	supported_aa_modes.push_back(rtt::TAA_SSAA_2_FXAA);
	
	bool chosen_aa_mode_supported = false;
	for(const auto& aa_mode : supported_aa_modes) {
		if(aa_mode == config.anti_aliasing) chosen_aa_mode_supported = true;
	}

	// if the chosen anti-aliasing mode isn't supported, use the next best one
	if(!chosen_aa_mode_supported) {
		config.anti_aliasing = supported_aa_modes.back();
		a2e_error("your chosen anti-aliasing mode isn't supported by your graphic card - using \"%s\" instead!", rtt::TEXTURE_ANTI_ALIASING_STR[config.anti_aliasing]);
	}
	else a2e_debug("using \"%s\" anti-aliasing", rtt::TEXTURE_ANTI_ALIASING_STR[config.anti_aliasing]);

	// check anisotropic
	if(config.anisotropic > exts->get_max_anisotropic_filtering()) {
		config.anisotropic = exts->get_max_anisotropic_filtering();
		a2e_error("your chosen anisotropic-filtering value isn't supported by your graphic card - using \"%u\" instead!", config.anisotropic);
	}
	else a2e_debug("using \"%ux\" anisotropic-filtering", config.anisotropic);
	
	// create texture manager and render to texture object
	t = new texman(f, u, exts, datapath, config.anisotropic);
	r = new rtt(this, exts, (unsigned int)config.width, (unsigned int)config.height);
	
	// if GL_RENDERER is that damn m$ gdi driver, exit a2e 
	// no official support for this crappy piece of software 
	if(strcmp((const char*)glGetString(GL_RENDERER), "GDI Generic") == 0) {
		a2e_error("A2E doesn't support the MS GDI Generic driver!\nGo and install one of these (that match your grapic card):\nhttp://www.ati.com  http://www.nvidia.com  http://www.intel.com");
		SDL_Delay(10000);
		exit(1);
	}

	// set standard texture filtering + anisotropic filtering
	t->set_filtering(config.filtering);

	// get standard (sdl internal) cursor and create cursor data
	standard_cursor = SDL_GetCursor();
	cursors["STANDARD"] = standard_cursor;

	// seed
	const unsigned int rseed = ((unsigned int)time(nullptr)/SDL_GetTicks())*((unsigned int)time(nullptr)%SDL_GetTicks());
	srand(rseed >> 1);
	
	// init/create shaders, init gfx
	shd = new shader(this);
	gfx2d::init(this);
	
	// draw the loading screen/image
	start_draw();
	start_2d_draw();
	a2e_texture load_tex = t->add_texture(data_path("loading.png"), texture_object::TF_LINEAR, 0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	const uint2 load_tex_draw_size(load_tex->width/2, load_tex->height/2);
	const uint2 img_offset((unsigned int)config.width/2 - load_tex_draw_size.x/2,
						   (unsigned int)config.height/2 - load_tex_draw_size.y/2);
	gfx2d::set_blend_mode(gfx2d::BLEND_MODE::PRE_MUL);
	gfx2d::draw_rectangle_texture(rect(img_offset.x, img_offset.y,
									   img_offset.x + load_tex_draw_size.x,
									   img_offset.y + load_tex_draw_size.y),
								  load_tex->tex(),
								  coord(0.0f, 1.0f), coord(1.0f, 0.0f));
	stop_2d_draw();
	stop_draw();
	
	// ctx is acquired 2 times, so release it 2 times
	release_gl_context();
	
	//
	acquire_gl_context();
	
	// create scene
	sce = new scene(this);
	
	// create gui
	ui = new gui(this);
	
#if !defined(A2E_NO_OPENCL)
	// init opencl
	ocl->init(false, config.opencl_platform);
#endif
	
	release_gl_context();
}

/*! sets the windows width
 *  @param width the window width
 */
void engine::set_width(unsigned int width) {
	config.width = width;
	e->add_event(EVENT_TYPE::WINDOW_RESIZE,
				 make_shared<window_resize_event>(SDL_GetTicks(), size2(config.width, config.height)));
}

/*! sets the window height
 *  @param height the window height
 */
void engine::set_height(unsigned int height) {
	config.height = height;
	e->add_event(EVENT_TYPE::WINDOW_RESIZE,
				 make_shared<window_resize_event>(SDL_GetTicks(), size2(config.width, config.height)));
}

/*! starts drawing the window
 */
void engine::start_draw() {
	acquire_gl_context();
	
	// draws ogl stuff
	glBindFramebuffer(GL_FRAMEBUFFER, A2E_DEFAULT_FRAMEBUFFER);
	glViewport(0, 0, (unsigned int)config.width, (unsigned int)config.height);
	
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
			a2e_error("OpenGL error: invalid enum!");
			break;
		case GL_INVALID_VALUE:
			a2e_error("OpenGL error: invalid value!");
			break;
		case GL_INVALID_OPERATION:
			a2e_error("OpenGL error: invalid operation!");
			break;
		case GL_OUT_OF_MEMORY:
			a2e_error("OpenGL error: out of memory!");
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			a2e_error("OpenGL error: invalid framebuffer operation!");
			break;
		default:
			a2e_error("unknown OpenGL error: %u!");
			break;
	}
	
	frame_time_sum += SDL_GetTicks() - frame_time_counter;

	// fps "limiter"
	if(config.fps_limit != 0) SDL_Delay((unsigned int)config.fps_limit);

	// handle fps count
	fps_counter++;
	if(SDL_GetTicks() - fps_time > 1000) {
		fps = fps_counter;
		new_fps_count = true;
		fps_counter = 0;
		fps_time = SDL_GetTicks();
		
		frame_time = (float)frame_time_sum / (float)fps;
		frame_time_sum = 0;
	}
	frame_time_counter = SDL_GetTicks();
	
	// check for shader/kernel reload (this is safe to do here)
	if(AtomicGet(&reload_shaders_flag) == 1) {
		AtomicSet(&reload_shaders_flag, 0);
		glFlush();
		glFinish();
		shd->reload_shaders();
	}
	if(AtomicGet(&reload_kernels_flag) == 1) {
		AtomicSet(&reload_kernels_flag, 0);
		glFlush();
		glFinish();
#if !defined(A2E_NO_OPENCL)
		ocl->flush();
		ocl->finish();
		ocl->reload_kernels();
#endif
	}
	
	release_gl_context();
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

/*! sets the window caption
 *  @param caption the window caption
 */
void engine::set_caption(const char* caption) {
	SDL_SetWindowTitle(config.wnd, caption);
}

/*! returns the window caption
 */
const char* engine::get_caption() {
	return SDL_GetWindowTitle(config.wnd);
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
	glDepthFunc(GL_LEQUAL);
	// enable backface culling
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
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
	glViewport(0, 0, (GLsizei)config.width, (GLsizei)config.height);

	// projection matrix
	// set perspective with fov (default = 72) and near/far plane value (default = 1.0f/1000.0f)
	projection_matrix.perspective(config.fov, float(config.width) / float(config.height),
								  config.near_far_plane.x, config.near_far_plane.y);

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
	start_2d_draw((unsigned int)config.width, (unsigned int)config.height);
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
	glDisable(GL_BLEND);
}

/*! stops drawing the 2d elements
 */
void engine::stop_2d_draw() {
	pop_projection_matrix();
	pop_modelview_matrix();
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE); // TODO: GL3, remove again
}

/*! sets the cursors visibility to state
 *  @param state the cursor visibility state
 */
void engine::set_cursor_visible(bool state) {
	engine::cursor_visible = state;
	SDL_ShowCursor(engine::cursor_visible);
}

/*! returns the cursor visibility stateo
 */
bool engine::get_cursor_visible() {
	return engine::cursor_visible;
}

/*! sets the fps limit (max. fps = 1000 / ms)
 *! note that a value of 0 increases the cpu usage to 100%
 *  @param ms how many milliseconds the engine should "sleep" after a frame is rendered
 */
void engine::set_fps_limit(unsigned int ms) {
	config.fps_limit = ms;
}

/*! returns how many milliseconds the engine is "sleeping" after a frame is rendered
 */
unsigned int engine::get_fps_limit() {
	return (unsigned int)config.fps_limit;
}

/*! returns the type of the initialization (0 = GRAPHICAL, 1 = CONSOLE)
 */
unsigned int engine::get_init_mode() {
	return engine::mode;
}

/*! returns a pointer to the core class
 */
core* engine::get_core() {
	return engine::c;
}

/*! returns a pointer to the file_io class
 */
file_io* engine::get_file_io() {
	return engine::f;
}

/*! returns a pointer to the file_io class
 */
event* engine::get_event() {
	return engine::e;
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

/*! returns the xml class
 */
xml* engine::get_xml() {
	return engine::x;
}

/*! returns the rtt class
 */
rtt* engine::get_rtt() {
	return engine::r;
}

/*! returns the unicode class
 */
unicode* engine::get_unicode() {
	return engine::u;
}

/*! returns the opencl class
 */
opencl* engine::get_opencl() {
	return engine::ocl;
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

/*! sets the data path
 *  @param data_path the data path
 */
void engine::set_data_path(const char* data_path) {
	engine::datapath = data_path;
}

/*! returns the data path
 */
string engine::get_data_path() const {
	return datapath;
}

/*! returns the call path
 */
string engine::get_call_path() const {
	return callpath;
}

/*! returns the shader path
 */
string engine::get_shader_path() const {
	return shaderpath;
}

/*! returns the kernel path
 */
string engine::get_kernel_path() const {
	return kernelpath;
}

/*! returns data path + str
 *  @param str str we want to "add" to the data path
 */
string engine::data_path(const string& str) const {
	if(str.length() == 0) return datapath;
	return datapath + str;
}

/*! returns data path + shader path + str
 *  @param str str we want to "add" to the data + shader path
 */
string engine::shader_path(const string& str) const {
	if(str.length() == 0) return datapath + shaderpath;
	return datapath + shaderpath + str;
}

/*! returns data path + kernel path + str
 *  @param str str we want to "add" to the data + kernel path
 */
string engine::kernel_path(const string& str) const {
	if(str.length() == 0) return datapath + kernelpath;
	return datapath + kernelpath + str;
}

/*! strips the data path from a string
 *  @param str str we want strip the data path from
 */
string engine::strip_data_path(const string& str) const {
	if(str.length() == 0) return "";
	return core::find_and_replace(str, datapath, "");
}

engine::server_data* engine::get_server_data() {
	return &config.server;
}

engine::client_data* engine::get_client_data() {
	return &config.client;
}

void engine::load_ico(const char* ico) {
	SDL_SetWindowIcon(config.wnd, IMG_Load(data_path(ico).c_str()));
}

texture_object::TEXTURE_FILTERING engine::get_filtering() {
	return config.filtering;
}

size_t engine::get_anisotropic() {
	return config.anisotropic;
}

rtt::TEXTURE_ANTI_ALIASING engine::get_anti_aliasing() {
	return config.anti_aliasing;
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

unsigned int engine::get_fps() {
	new_fps_count = false;
	return engine::fps;
}

float engine::get_frame_time() {
	return engine::frame_time;
}

bool engine::is_new_fps_count() {
	return engine::new_fps_count;
}

SDL_Cursor* engine::add_cursor(const char* name, const char** raw_data, unsigned int xsize, unsigned int ysize, unsigned int hotx, unsigned int hoty) {
	if(cursors.count(name)) {
		a2e_error("cursor with such a name (%s) already exists!", name);
		return nullptr;
	}

	if((xsize != 16 || ysize != 16) && (xsize != 32 || ysize != 32)) {
		a2e_error("invalid cursor size (%ux%u) - must be either 16x16 or 32x32!", xsize, ysize);
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
		a2e_error("no cursor with such a name (%s) exists!", name);
		return nullptr;
	}
	return cursors[name];
}

bool engine::get_fullscreen() const {
	return config.fullscreen;
}

bool engine::get_vsync() const {
	return config.vsync;
}

bool engine::get_stereo() const {
	return config.stereo;
}

unsigned int engine::get_width() const {
	return (unsigned int)config.width;
}

unsigned int engine::get_height() const {
	return (unsigned int)config.height;
}

unsigned int engine::get_key_repeat() {
	return (unsigned int)config.key_repeat;
}

unsigned int engine::get_ldouble_click_time() {
	return (unsigned int)config.ldouble_click_time;
}

unsigned int engine::get_mdouble_click_time() {
	return (unsigned int)config.mdouble_click_time;
}

unsigned int engine::get_rdouble_click_time() {
	return (unsigned int)config.rdouble_click_time;
}

string* engine::get_disabled_extensions() {
	return &config.disabled_extensions;
}

string* engine::get_force_device() {
	return &config.force_device;
}

string* engine::get_force_vendor() {
	return &config.force_vendor;
}

SDL_Window* engine::get_window() const {
	return config.wnd;
}

size_t engine::get_inferred_scale() const {
	return config.inferred_scale;
}

const string engine::get_version() const {
	return A2E_VERSION_STRING;
}

void engine::swap() {
	SDL_GL_SwapWindow(config.wnd);
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
	AtomicSet(&reload_shaders_flag, 1);
}

void engine::reload_kernels() {
	AtomicSet(&reload_kernels_flag, 1);
}

const float& engine::get_fov() const {
	return config.fov;
}

const float2& engine::get_near_far_plane() const {
	return config.near_far_plane;
}

const xml::xml_doc& engine::get_config_doc() const {
	return config_doc;
}

void engine::acquire_gl_context() {
	// note: the context lock is recursive, so one thread can lock
	// it multiple times. however, SDL_GL_MakeCurrent should only
	// be called once (this is the purpose of ctx_active_locks).
	config.ctx_lock.lock();
	// note: not a race, since there can only be one active gl thread
	const int cur_active_locks = AtomicFetchThenIncrement(&config.ctx_active_locks);
	if(cur_active_locks == 0 &&
	   SDL_GL_MakeCurrent(config.wnd, config.ctx) != 0) {
		a2e_error("couldn't make gl context current: %s!", SDL_GetError());
		return;
	}
#if defined(A2E_IOS)
	glBindFramebuffer(GL_FRAMEBUFFER, 1);
#endif
}

void engine::release_gl_context() {
	// only call SDL_GL_MakeCurrent will nullptr, when this is the last lock
	const int cur_active_locks = AtomicFetchThenDecrement(&config.ctx_active_locks);
	if(cur_active_locks == 1 &&
	   SDL_GL_MakeCurrent(config.wnd, nullptr) != 0) {
		a2e_error("couldn't release current gl context: %s!", SDL_GetError());
		return;
	}
	config.ctx_lock.unlock();
}

bool engine::window_event_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(type == EVENT_TYPE::WINDOW_RESIZE) {
		const window_resize_event& evt = (const window_resize_event&)*obj;
		config.width = evt.size.x;
		config.height = evt.size.y;
		acquire_gl_context();
		resize_window();
		release_gl_context();
	}
	return true;
}
