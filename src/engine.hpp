/*
 *  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2013 Florian Ziesche
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
#include "floor/floor.hpp"
#include "core/core.hpp"
#include "core/file_io.hpp"
#include "core/event.hpp"
#include "rendering/texman.hpp"
#include "rendering/extensions.hpp"
#include "core/xml.hpp"
#include "rendering/rtt.hpp"
#include "core/vector3.hpp"
#include "core/matrix4.hpp"
#include "core/unicode.hpp"

#define A2M_VERSION 2

/*! @class engine
 *  @brief main engine
 */

// the initialization mode is used to determine if we should load
// or compute graphical stuff like textures or shaders
enum class INIT_MODE : unsigned int {
	GRAPHICAL,
	CONSOLE
};

class shader;
class opencl_base;
class gui;
class scene;
class A2E_API engine {
public:
	engine() = delete;
	~engine() = delete;
	
	// graphic control functions
	void init(const char* callpath, const char* datapath,
			  const bool console_only = false, const string config_name = "config.xml",
			  const char* ico = nullptr);
	void destroy();
	
	void start_draw();
	void stop_draw();
	void start_2d_draw();
	void start_2d_draw(const unsigned int width, const unsigned int height);
	void stop_2d_draw();
	void push_ogl_state();
	void pop_ogl_state();
	void init_gl();
	void resize_window();
	void swap();
	const string get_version() const;
	
	// class return functions
	texman* get_texman();
	ext* get_ext();
	rtt* get_rtt();
	shader* get_shader();
	gui* get_gui();
	scene* get_scene();

	// miscellaneous control functions
	SDL_Cursor* add_cursor(const char* name, const char** raw_data, unsigned int xsize, unsigned int ysize, unsigned int hotx, unsigned int hoty);
	void set_cursor(SDL_Cursor* cursor);
	SDL_Cursor* get_cursor(const char* name);
	
	string get_shader_path() const;
	string shader_path(const string& str) const;
	
	void reload_shaders();
	
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
	
	const INIT_MODE& get_init_mode();
	
	// gui
	const rtt::TEXTURE_ANTI_ALIASING& get_ui_anti_aliasing() const;
	
	// input
	unsigned int get_key_repeat() const;
	unsigned int get_ldouble_click_time() const;
	unsigned int get_mdouble_click_time() const;
	unsigned int get_rdouble_click_time() const;
	
	// sleep / fps limit
	void set_fps_limit(unsigned int ms);
	unsigned int get_fps_limit() const;
	
	// graphic
	TEXTURE_FILTERING get_filtering() const;
	size_t get_anisotropic() const;
	rtt::TEXTURE_ANTI_ALIASING get_anti_aliasing() const;
	
	void set_filtering(const TEXTURE_FILTERING& filtering);
	void set_anisotropic(const size_t& anisotropic);
	void set_anti_aliasing(const rtt::TEXTURE_ANTI_ALIASING& anti_aliasing);
	
	// graphic device
	const string& get_disabled_extensions() const;
	const string& get_force_device() const;
	const string& get_force_vendor() const;
	
	// inferred rendering
	float get_upscaling() const;
	float get_geometry_light_scaling() const;
	
	void set_upscaling(const float& factor);
	void set_geometry_light_scaling(const float& factor);

protected:
	texman* t = nullptr;
	ext* exts = nullptr;
	rtt* r = nullptr;
	shader* shd = nullptr;
	gui* ui = nullptr;
	scene* sce = nullptr;
	event* evt = nullptr;
	
	void load_ico(const char* ico);
	
	struct engine_config {
		// gui
		size_t ui_anti_aliasing = 8;
		rtt::TEXTURE_ANTI_ALIASING ui_anti_aliasing_enum = rtt::TEXTURE_ANTI_ALIASING::MSAA_8;
		
		// input
		size_t key_repeat = 200;
		size_t ldouble_click_time = 200;
		size_t mdouble_click_time = 200;
		size_t rdouble_click_time = 200;
		
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
	string shaderpath;

	// screen info variables
	INIT_MODE mode;
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
	
	// cursor
	SDL_Cursor* standard_cursor;
	map<string, SDL_Cursor*> cursors;
	unsigned char* cursor_data;
	unsigned char* cursor_mask;
	unsigned char cursor_data16[2*16];
	unsigned char cursor_mask16[2*16];
	unsigned char cursor_data32[4*32];
	unsigned char cursor_mask32[4*32];
	
	// window event handlers
	event::handler* window_handler;
	bool window_event_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	
	// misc
	atomic<bool> reload_shaders_flag { false };
	GLuint global_vao;

};

#endif
