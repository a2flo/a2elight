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

#ifndef __A2E_ENGINE_HPP__
#define __A2E_ENGINE_HPP__

#include "global.hpp"
#include <floor/floor/floor.hpp>
#include <floor/core/core.hpp>
#include <floor/core/file_io.hpp>
#include <floor/core/event.hpp>
#include "rendering/texman.hpp"
#include "rendering/extensions.hpp"
#include <floor/core/xml.hpp>
#include "rendering/rtt.hpp"
#include <floor/math/vector_lib.hpp>
#include <floor/math/matrix4.hpp>
#include <floor/core/unicode.hpp>

#define A2M_VERSION 2

class shader;
class gui;
class scene;

//! main engine
class engine {
public:
	engine() = delete;
	~engine() = delete;
	
	// the initialization mode is used to determine if we should load
	// or compute graphical stuff like textures or shaders
	enum class INIT_MODE : unsigned int {
		GRAPHICAL,
		CONSOLE
	};
	
	// graphic control functions
	static void init(const char* callpath, const char* datapath,
					 const bool console_only = false, const string config_name = "config.json",
					 const char* ico = nullptr);
	static void destroy();
	
	static void start_draw();
	static void stop_draw();
	static void start_2d_draw();
	static void start_2d_draw(const unsigned int width, const unsigned int height);
	static void stop_2d_draw();
	static void push_ogl_state();
	static void pop_ogl_state();
	static void init_gl();
	static void resize_window();
	static const string get_version();
	
	// class return functions
	static texman* get_texman();
	static ext* get_ext();
	static rtt* get_rtt();
	static shader* get_shader();
	static gui* get_gui();
	static scene* get_scene();
	static xml* get_xml();

	// miscellaneous control functions
	static SDL_Cursor* add_cursor(const char* name, const char** raw_data, unsigned int xsize, unsigned int ysize, unsigned int hotx, unsigned int hoty);
	static void set_cursor(SDL_Cursor* cursor);
	static SDL_Cursor* get_cursor(const char* name);
	
	static string get_shader_path();
	static string shader_path(const string& str);
	
	static void reload_shaders();
	
	// misc position/rotation/matrix functions
	static void push_projection_matrix();
	static void pop_projection_matrix();
	static void push_modelview_matrix();
	static void pop_modelview_matrix();
	static matrix4f* get_projection_matrix();
	static matrix4f* get_modelview_matrix();
	static matrix4f* get_mvp_matrix();
	static matrix4f* get_translation_matrix();
	static matrix4f* get_rotation_matrix();
	static void set_position(float xpos, float ypos, float zpos);
	static float3* get_position(); //! shouldn't be used outside of the engine, use camera class function instead
	static void set_rotation(float xrot, float yrot);
	static float3* get_rotation(); //! shouldn't be used outside of the engine, use camera class function instead
	
	static const INIT_MODE& get_init_mode();
	
	// gui
	static const rtt::TEXTURE_ANTI_ALIASING& get_ui_anti_aliasing();
	
	// sleep / fps limit
	static void set_fps_limit(unsigned int ms);
	static unsigned int get_fps_limit();
	
	// graphic
	static TEXTURE_FILTERING get_filtering();
	static size_t get_anisotropic();
	static rtt::TEXTURE_ANTI_ALIASING get_anti_aliasing();
	
	static void set_filtering(const TEXTURE_FILTERING& filtering);
	static void set_anisotropic(const size_t& anisotropic);
	static void set_anti_aliasing(const rtt::TEXTURE_ANTI_ALIASING& anti_aliasing);
	
	// graphic device
	static const string& get_disabled_extensions();
	static const string& get_force_device();
	static const string& get_force_vendor();
	
	// inferred rendering
	static float get_upscaling();
	static float get_geometry_light_scaling();
	
	static void set_upscaling(const float& factor);
	static void set_geometry_light_scaling(const float& factor);

protected:
	static texman* t;
	static ext* exts;
	static rtt* r;
	static shader* shd;
	static gui* ui;
	static scene* sce;
	static event* evt;
	static xml* x;
	
	static void load_ico(const char* ico);
	
	static struct engine_config {
		// gui
		size_t ui_anti_aliasing = 8;
		rtt::TEXTURE_ANTI_ALIASING ui_anti_aliasing_enum = rtt::TEXTURE_ANTI_ALIASING::MSAA_8;
		
		// sleep / fps limit
		size_t fps_limit = 0;
		
		// graphic
		TEXTURE_FILTERING filtering = TEXTURE_FILTERING::POINT;
		rtt::TEXTURE_ANTI_ALIASING anti_aliasing = rtt::TEXTURE_ANTI_ALIASING::NONE;
		size_t anisotropic = 0;
		
		// graphic device
		string disabled_extensions = "";
		string force_device = "";
		string force_vendor = "";
		
		// inferred rendering
		float upscaling = 1.0f;
		float geometry_light_scaling = 1.0f;
	} config;
	
	// path variables
	static string shaderpath;

	// screen info variables
	static INIT_MODE init_mode;
	static vector<rtt::TEXTURE_ANTI_ALIASING> supported_aa_modes;

	// transformation/positioning/rotation
	static float3 position;
	static float3 rotation;
	static matrix4f projection_matrix;
	static matrix4f modelview_matrix;
	static matrix4f mvp_matrix;
	static matrix4f translation_matrix;
	static matrix4f rotation_matrix;
	static deque<matrix4f*> projm_stack;
	static deque<matrix4f*> mvm_stack;
	
	// pushed gl state
	static GLint pushed_matrix_mode;
	static GLenum pushed_blend_src, pushed_blend_dst;
	static GLenum pushed_blend_src_rgb, pushed_blend_src_alpha;
	static GLenum pushed_blend_dst_rgb, pushed_blend_dst_alpha;
	static deque<matrix4f*> pushed_matrices;
	
	// cursor
	static SDL_Cursor* standard_cursor;
	static map<string, SDL_Cursor*> cursors;
	static unsigned char* cursor_data;
	static unsigned char* cursor_mask;
	static unsigned char cursor_data16[2*16];
	static unsigned char cursor_mask16[2*16];
	static unsigned char cursor_data32[4*32];
	static unsigned char cursor_mask32[4*32];
	
	// window event handlers
	static event::handler* window_handler;
	static bool window_event_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	
	// misc
	static atomic<bool> reload_shaders_flag;
	static GLuint global_vao;

};

#endif
