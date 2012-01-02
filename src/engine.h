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

#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "global.h"
#include "core/core.h"
#include "core/file_io.h"
#include "gui/event.h"
#include "rendering/gfx.h"
#include "rendering/texman.h"
#include "rendering/extensions.h"
#include "core/xml.h"
#include "rendering/rtt.h"
#include "core/vector3.h"
#include "core/matrix4.h"
#include "gui/unicode.h"

#define A2M_VERSION 2

/*! @class engine
 *  @brief main engine
 */

class shader;
class opencl;
class A2E_API engine {
public:
	engine(const char* callpath_, const char* datapath_);
	~engine();
	
	// graphic control functions
	void init(const char* ico = NULL);
	void init(bool console, unsigned int width = 640, unsigned int height = 480,
			  bool fullscreen = false, bool vsync = false, const char* ico = NULL);
	void set_width(unsigned int width);
	void set_height(unsigned int height);
	void start_draw();
	void stop_draw();
	void start_2d_draw();
	void start_2d_draw(const unsigned int width, const unsigned int height, const bool fbo);
	void stop_2d_draw();
	void push_ogl_state();
	void pop_ogl_state();
	bool init_gl();
	bool resize_window();
	void swap();
	const string get_version() const;
	
	// class return functions
	core* get_core();
	file_io* get_file_io();
	event* get_event();
	gfx* get_gfx();
	texman* get_texman();
	ext* get_ext();
	xml* get_xml();
	rtt* get_rtt();
	unicode* get_unicode();
	opencl* get_opencl();
	shader* get_shader();
	
	// the initialization mode is used to determine if we should load
	// or compute graphical stuff like textures or shaders
	enum INIT_MODE {
	    GRAPHICAL,
	    CONSOLE
	};
	
	struct server_data {
		unsigned short int port;
		unsigned int max_clients;
	};
	
	struct client_data {
		string server_name;
		unsigned short int port;
		unsigned short int lis_port;
		string client_name;
	};

	// miscellaneous control functions
	void set_caption(const char* caption);
	const char* get_caption();

	void set_cursor_visible(bool state);
	bool get_cursor_visible();
	SDL_Cursor* add_cursor(const char* name, const char** raw_data, unsigned int xsize, unsigned int ysize, unsigned int hotx, unsigned int hoty);
	void set_cursor(SDL_Cursor* cursor);
	SDL_Cursor* get_cursor(const char* name);
	
	void set_data_path(const char* data_path = "../data/");
	string get_data_path() const;
	string get_call_path() const;
	string get_shader_path() const;
	string get_kernel_path() const;
	string data_path(const string& str) const;
	string shader_path(const string& str) const;
	string kernel_path(const string& str) const;
	string strip_data_path(const string& str) const;
	
	void reload_shaders();
	void reload_kernels();
	
	// misc position/rotation/matrix functions
	void push_projection_matrix();
	void pop_projection_matrix();
	void push_modelview_matrix();
	void pop_modelview_matrix();
	matrix4f* get_projection_matrix();
	matrix4f* get_modelview_matrix();
	matrix4f* get_mvp_matrix();
	matrix4f* get_translation_matrix();
	matrix4f* get_rotation_matrix();
	void set_position(float xpos, float ypos, float zpos);
	float3* get_position(); //! shouldn't be used outside of the engine, use camera class function instead
	void set_rotation(float xrot, float yrot);
	float3* get_rotation(); //! shouldn't be used outside of the engine, use camera class function instead
	
	// fps functions
	unsigned int get_fps();
	float get_frame_time();
	bool is_new_fps_count();
	
	unsigned int get_init_mode();

	// config functions
	const xml::xml_doc& get_config_doc() const;
	
	// screen/window
	SDL_Window* get_window();
	unsigned int get_width();
	unsigned int get_height();
	bool get_fullscreen();
	bool get_vsync();
	bool get_stereo();
	
	// projection
	const float& get_fov() const;
	const float2& get_near_far_plane() const;
	
	// gui
	// TODO
	
	// input
	unsigned int get_key_repeat();
	unsigned int get_ldouble_click_time();
	unsigned int get_mdouble_click_time();
	unsigned int get_rdouble_click_time();
	
	// sleep / fps limit
	void set_fps_limit(unsigned int ms);
	unsigned int get_fps_limit();
	
	// thread
	size_t get_thread_count();
	
	// server / client
	server_data* get_server_data();
	client_data* get_client_data();
	
	// graphic
	texture_object::TEXTURE_FILTERING get_filtering();
	size_t get_anisotropic();
	rtt::TEXTURE_ANTI_ALIASING get_anti_aliasing();
	
	// graphic device
	string* get_disabled_extensions();
	string* get_force_device();
	string* get_force_vendor();
	
	// inferred rendering
	size_t get_inferred_scale() const;

protected:
	core* c;
	file_io* f;
	event* e;
	gfx* g;
	texman* t;
	ext* exts;
	xml* x;
	rtt* r;
	unicode* u;
	opencl* ocl;
	shader* shd;
	
	// actual engine constructor
	void create();
	void load_ico(const char* ico);
	
	struct engine_config {
		// screen
		size_t width, height;
		bool fullscreen, vsync, stereo;
		
		// projection
		float fov;
		float2 near_far_plane;
		
		// gui
		// TODO
		
		// input
		size_t key_repeat;
		size_t ldouble_click_time;
		size_t mdouble_click_time;
		size_t rdouble_click_time;
		
		// sleep / fps limit
		size_t fps_limit;
		
		// server
		server_data server;
		
		// client
		client_data client;
		
		// graphic
		texture_object::TEXTURE_FILTERING filtering;
		rtt::TEXTURE_ANTI_ALIASING anti_aliasing;
		size_t anisotropic;
		
		// graphic device
		string disabled_extensions;
		string force_device;
		string force_vendor;
		
		// inferred rendering
		// 0: 50%, 1: 62.5%, 2: 75%, 3: 87.5%, 4: 100%
		ssize_t inferred_scale;
		
		// opencl
		size_t opencl_platform;
		bool clear_cache;

		// sdl
		SDL_Window* wnd;
		SDL_GLContext ctx;
		unsigned int flags;
		
		engine_config() :
		width(640), height(480),
		fullscreen(false), vsync(false), stereo(false),
		fov(72.0f), near_far_plane(1.0f, 1000.0f),
		key_repeat(200), ldouble_click_time(200), mdouble_click_time(200), rdouble_click_time(200),
		fps_limit(0),
		server(), client(),
		filtering(texture_object::TF_POINT), anti_aliasing(rtt::TAA_NONE), anisotropic(0),
		disabled_extensions(""), force_device(""), force_vendor(""),
		inferred_scale(4),
		opencl_platform(0), clear_cache(false),
		wnd(NULL), ctx(NULL), flags(0)
		{}
	};
	engine_config config;
	xml::xml_doc config_doc;
	
	// path variables
	string datapath;
	string rel_datapath;
	string callpath;
	string shaderpath;
	string kernelpath;

	// screen info variables
	unsigned int mode;
	vector<rtt::TEXTURE_ANTI_ALIASING> supported_aa_modes;

	// transformation/positioning/rotation
	float3 position;
	float3 rotation;
	matrix4f projection_matrix;
	matrix4f modelview_matrix;
	matrix4f mvp_matrix;
	matrix4f translation_matrix;
	matrix4f rotation_matrix;
	deque<matrix4f*> projm_stack;
	deque<matrix4f*> mvm_stack;
	
	// pushed gl state
	GLint pushed_matrix_mode;
	GLint pushed_blend_src, pushed_blend_dst;
	GLint pushed_blend_src_rgb, pushed_blend_src_alpha;
	GLint pushed_blend_dst_rgb, pushed_blend_dst_alpha;
	deque<matrix4f*> pushed_matrices;

	// fps counting
	unsigned int fps;
	unsigned int fps_counter;
	unsigned int fps_time;
	float frame_time;
	unsigned int frame_time_sum;
	unsigned int frame_time_counter;
	bool new_fps_count;
	
	// cursor
	bool cursor_visible;
	SDL_Cursor* standard_cursor;
	map<string, SDL_Cursor*> cursors;
	unsigned char* cursor_data;
	unsigned char* cursor_mask;
	unsigned char cursor_data16[2*16];
	unsigned char cursor_mask16[2*16];
	unsigned char cursor_data32[4*32];
	unsigned char cursor_mask32[4*32];
	
	// misc
	atomic_t reload_shaders_flag;
	atomic_t reload_kernels_flag;

};

#endif
