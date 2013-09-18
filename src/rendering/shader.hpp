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

#ifndef __A2E_SHADER_HPP__
#define __A2E_SHADER_HPP__

#include "global.hpp"

#include "engine.hpp"
#include "rendering/extensions.hpp"
#include "rendering/rtt.hpp"
#include "core/xml.hpp"
#include "rendering/renderer/a2e_shader.hpp"
#include "rendering/renderer/shader_object.hpp"
#include "rendering/renderer/shader_base.hpp"
#if !defined(A2E_IOS)
#include "rendering/renderer/gl3/shader_gl3.hpp"
#else
#include "rendering/renderer/gles2/shader_gles2.hpp"
#endif

/*! @class shader
 *  @brief shader class
 */

class A2E_API shader {
protected:
	map<string, shader_object*> shaders;
	
public:
	shader();
	~shader();
	
	shader_object* add_shader_file(const string& identifier, ext::GLSL_VERSION glsl_version, const char* vname, const char* gname, const char* fname);
	shader_object* add_shader_src(const string& identifier, ext::GLSL_VERSION glsl_version, const char* vs_text, const char* gs_text, const char* fs_text);
	shader_object* add_shader_src(const string& identifier, const string& option, ext::GLSL_VERSION glsl_version, const char* vs_text, const char* gs_text, const char* fs_text);
	shader_object* get_shader_object(const string& identifier);
	
	bool add_a2e_shader(const string& identifier, const string& filename);
	
	template <class shader_type> shader_type get_shader(const string& identifier) const;
	// for convenience:
	gl3shader get_gl3shader(const string& identifier) const;
	
	// actually a rtt function, but put here, b/c it uses shaders (which aren't allowed in rtt class, b/c of class dependency)
	void copy_buffer(rtt::fbo* src_buffer, rtt::fbo* dest_buffer, unsigned int src_attachment = 0, unsigned int dest_attachment = 0);

	void set_gui_shader_rendering(bool state);
	bool is_gui_shader_rendering();

	a2e_shader* get_a2e_shader();

	//! reloads all internal and external shaders that were added via add_a2e_shader
	//! note: this invalidates _all_ shaders!
	void reload_shaders();

protected:
	ext* exts;
	rtt* r;
	xml* x;
	a2e_shader* a2e_shd;

	GLenum copy_draw_buffer[1];
	bool gui_shader_rendering;
	
	bool load_internal_shaders();
	
	map<string, string> external_shaders;
	
	void log_pretty_print(const char* log, const char* code) const;
	
};

template <> gl3shader shader::get_shader(const string& identifier) const;

#endif
