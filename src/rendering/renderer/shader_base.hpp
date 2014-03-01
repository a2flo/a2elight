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

#ifndef __A2E_SHADER_BASE_HPP__
#define __A2E_SHADER_BASE_HPP__

#include "global.hpp"
#include "engine.hpp"
#include "rendering/extensions.hpp"
#include "rendering/rtt.hpp"
#include "rendering/texture_object.hpp"
#include "rendering/renderer/shader_object.hpp"

#if !defined(FLOOR_IOS)

// OpenGL 3.2 Core
#define A2E_GL_SHADER_TYPES(F) \
F(GL_FLOAT) \
F(GL_FLOAT_VEC2) \
F(GL_FLOAT_VEC3) \
F(GL_FLOAT_VEC4) \
F(GL_INT) \
F(GL_INT_VEC2) \
F(GL_INT_VEC3) \
F(GL_INT_VEC4) \
F(GL_UNSIGNED_INT) \
F(GL_UNSIGNED_INT_VEC2) \
F(GL_UNSIGNED_INT_VEC3) \
F(GL_UNSIGNED_INT_VEC4) \
F(GL_BOOL) \
F(GL_BOOL_VEC2) \
F(GL_BOOL_VEC3) \
F(GL_BOOL_VEC4) \
F(GL_FLOAT_MAT2) \
F(GL_FLOAT_MAT3) \
F(GL_FLOAT_MAT4) \
F(GL_FLOAT_MAT2x3) \
F(GL_FLOAT_MAT2x4) \
F(GL_FLOAT_MAT3x2) \
F(GL_FLOAT_MAT3x4) \
F(GL_FLOAT_MAT4x2) \
F(GL_FLOAT_MAT4x3) \
F(GL_SAMPLER_1D) \
F(GL_SAMPLER_2D) \
F(GL_SAMPLER_3D) \
F(GL_SAMPLER_CUBE) \
F(GL_SAMPLER_CUBE_SHADOW) \
F(GL_SAMPLER_1D_SHADOW) \
F(GL_SAMPLER_2D_SHADOW) \
F(GL_SAMPLER_1D_ARRAY) \
F(GL_SAMPLER_2D_ARRAY) \
F(GL_SAMPLER_1D_ARRAY_SHADOW) \
F(GL_SAMPLER_2D_ARRAY_SHADOW) \
F(GL_SAMPLER_BUFFER) \
F(GL_SAMPLER_2D_RECT) \
F(GL_SAMPLER_2D_RECT_SHADOW) \
F(GL_INT_SAMPLER_1D) \
F(GL_INT_SAMPLER_2D) \
F(GL_INT_SAMPLER_3D) \
F(GL_INT_SAMPLER_1D_ARRAY) \
F(GL_INT_SAMPLER_2D_ARRAY) \
F(GL_INT_SAMPLER_2D_RECT) \
F(GL_INT_SAMPLER_BUFFER) \
F(GL_INT_SAMPLER_CUBE) \
F(GL_UNSIGNED_INT_SAMPLER_1D) \
F(GL_UNSIGNED_INT_SAMPLER_2D) \
F(GL_UNSIGNED_INT_SAMPLER_3D) \
F(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY) \
F(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY) \
F(GL_UNSIGNED_INT_SAMPLER_2D_RECT) \
F(GL_UNSIGNED_INT_SAMPLER_BUFFER) \
F(GL_UNSIGNED_INT_SAMPLER_CUBE) \
F(GL_SAMPLER_2D_MULTISAMPLE) \
F(GL_INT_SAMPLER_2D_MULTISAMPLE) \
F(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE) \
F(GL_SAMPLER_2D_MULTISAMPLE_ARRAY) \
F(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY) \
F(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY)

#define A2E_GL_SHADER_SAMPLER_TYPES(F) \
F(GL_SAMPLER_1D) \
F(GL_SAMPLER_2D) \
F(GL_SAMPLER_3D) \
F(GL_SAMPLER_CUBE) \
F(GL_SAMPLER_1D_SHADOW) \
F(GL_SAMPLER_2D_SHADOW) \
F(GL_SAMPLER_1D_ARRAY) \
F(GL_SAMPLER_2D_ARRAY) \
F(GL_SAMPLER_1D_ARRAY_SHADOW) \
F(GL_SAMPLER_2D_ARRAY_SHADOW) \
F(GL_SAMPLER_CUBE_SHADOW) \
F(GL_SAMPLER_BUFFER) \
F(GL_SAMPLER_2D_RECT) \
F(GL_SAMPLER_2D_RECT_SHADOW) \
F(GL_INT_SAMPLER_1D) \
F(GL_INT_SAMPLER_2D) \
F(GL_INT_SAMPLER_3D) \
F(GL_INT_SAMPLER_1D_ARRAY) \
F(GL_INT_SAMPLER_2D_ARRAY) \
F(GL_INT_SAMPLER_2D_RECT) \
F(GL_INT_SAMPLER_BUFFER) \
F(GL_INT_SAMPLER_CUBE) \
F(GL_UNSIGNED_INT_SAMPLER_1D) \
F(GL_UNSIGNED_INT_SAMPLER_2D) \
F(GL_UNSIGNED_INT_SAMPLER_3D) \
F(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY) \
F(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY) \
F(GL_UNSIGNED_INT_SAMPLER_2D_RECT) \
F(GL_UNSIGNED_INT_SAMPLER_BUFFER) \
F(GL_UNSIGNED_INT_SAMPLER_CUBE) \
F(GL_SAMPLER_2D_MULTISAMPLE) \
F(GL_INT_SAMPLER_2D_MULTISAMPLE) \
F(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE) \
F(GL_SAMPLER_2D_MULTISAMPLE_ARRAY) \
F(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY) \
F(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY)

#else
#if defined(PLATFORM_X64)
// iOS / OpenGL ES 3.0 types

#define A2E_GL_SHADER_TYPES(F) \
F(GL_FLOAT) \
F(GL_FLOAT_VEC2) \
F(GL_FLOAT_VEC3) \
F(GL_FLOAT_VEC4) \
F(GL_INT) \
F(GL_INT_VEC2) \
F(GL_INT_VEC3) \
F(GL_INT_VEC4) \
F(GL_UNSIGNED_INT) \
F(GL_UNSIGNED_INT_VEC2) \
F(GL_UNSIGNED_INT_VEC3) \
F(GL_UNSIGNED_INT_VEC4) \
F(GL_BOOL) \
F(GL_BOOL_VEC2) \
F(GL_BOOL_VEC3) \
F(GL_BOOL_VEC4) \
F(GL_FLOAT_MAT2) \
F(GL_FLOAT_MAT3) \
F(GL_FLOAT_MAT4) \
F(GL_FLOAT_MAT2x3) \
F(GL_FLOAT_MAT2x4) \
F(GL_FLOAT_MAT3x2) \
F(GL_FLOAT_MAT3x4) \
F(GL_FLOAT_MAT4x2) \
F(GL_FLOAT_MAT4x3) \
F(GL_SAMPLER_2D) \
F(GL_SAMPLER_3D) \
F(GL_SAMPLER_CUBE) \
F(GL_SAMPLER_CUBE_SHADOW) \
F(GL_SAMPLER_2D_SHADOW) \
F(GL_SAMPLER_2D_ARRAY) \
F(GL_SAMPLER_2D_ARRAY_SHADOW) \
F(GL_INT_SAMPLER_2D) \
F(GL_INT_SAMPLER_3D) \
F(GL_INT_SAMPLER_CUBE) \
F(GL_INT_SAMPLER_2D_ARRAY) \
F(GL_UNSIGNED_INT_SAMPLER_2D) \
F(GL_UNSIGNED_INT_SAMPLER_3D) \
F(GL_UNSIGNED_INT_SAMPLER_CUBE) \
F(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY)

#define A2E_GL_SHADER_SAMPLER_TYPES(F) \
F(GL_SAMPLER_2D) \
F(GL_SAMPLER_3D) \
F(GL_SAMPLER_CUBE) \
F(GL_SAMPLER_CUBE_SHADOW) \
F(GL_SAMPLER_2D_SHADOW) \
F(GL_SAMPLER_2D_ARRAY) \
F(GL_SAMPLER_2D_ARRAY_SHADOW) \
F(GL_INT_SAMPLER_2D) \
F(GL_INT_SAMPLER_3D) \
F(GL_INT_SAMPLER_CUBE) \
F(GL_INT_SAMPLER_2D_ARRAY) \
F(GL_UNSIGNED_INT_SAMPLER_2D) \
F(GL_UNSIGNED_INT_SAMPLER_3D) \
F(GL_UNSIGNED_INT_SAMPLER_CUBE) \
F(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY)

#else
// iOS / OpenGL ES 2.0 types

#define A2E_GL_SHADER_TYPES(F) \
F(GL_FLOAT) \
F(GL_FLOAT_VEC2) \
F(GL_FLOAT_VEC3) \
F(GL_FLOAT_VEC4) \
F(GL_INT) \
F(GL_INT_VEC2) \
F(GL_INT_VEC3) \
F(GL_INT_VEC4) \
F(GL_BOOL) \
F(GL_BOOL_VEC2) \
F(GL_BOOL_VEC3) \
F(GL_BOOL_VEC4) \
F(GL_FLOAT_MAT2) \
F(GL_FLOAT_MAT3) \
F(GL_FLOAT_MAT4) \
F(GL_SAMPLER_2D) \
F(GL_SAMPLER_CUBE)

#define A2E_GL_SHADER_SAMPLER_TYPES(F) \
F(GL_SAMPLER_2D) \
F(GL_SAMPLER_CUBE)
#endif

#endif

//
#define A2E_SHADER_GET_UNIFORM_POSITION(name) (GLint)(get_uniform_position(name))
#define A2E_SHADER_GET_ATTRIBUTE_POSITION(name) (GLuint)(get_attribute_position(name))
#define A2E_SHADER_GET_BLOCK_POSITION(name) (GLuint)(get_block_position(name))

template<class shader_impl> class A2E_API shader_base {
public:
	shader_base(const shader_object& shd_obj_) : shd_obj(shd_obj_), cur_program(0), cur_option("#") {}
	virtual ~shader_base() {}
	
	// basic functions
	virtual void use() { cur_program = 0; }
	virtual void use(const size_t& program) { cur_program = program; }
	virtual void use(const string& option, const set<string> combiners = set<string> {}) = 0;
	virtual void disable() = 0;
	virtual size_t get_cur_program() const = 0;
	virtual const string& get_cur_option() const = 0;
	
	// functions for setting uniform variables
	template<typename arg1_type>
	void uniform(const char* name, const arg1_type& arg1) const;
	template<typename arg1_type>
	void uniform(const char* name, const arg1_type& arg1, const size_t& count) const;
	
	template<typename arg1_type, typename arg2_type>
	void uniform(const char* name, const arg1_type& arg1, const arg2_type& arg2) const;
	
	template<typename arg1_type, typename arg2_type, typename arg3_type>
	void uniform(const char* name, const arg1_type& arg1, const arg2_type& arg2, const arg3_type& arg3) const;
	
	template<typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type>
	void uniform(const char* name, const arg1_type& arg1, const arg2_type& arg2, const arg3_type& arg3, const arg4_type& arg4) const;
	
	// function for setting a texture (accepts direct gl ids (GLuint) and a2e texture objects)
	void texture(const char* name, const GLuint& tex, const GLenum texture_type = GL_TEXTURE_2D) const;
	void texture(const char* name, const a2e_texture& tex) const;
	
	// functions for setting attribute variables
	template<typename arg1_type>
	void attribute(const char* name, const arg1_type& arg1) const;
	
	template<typename arg1_type, typename arg2_type>
	void attribute(const char* name, const arg1_type& arg1, const arg2_type& arg2) const;
	
	template<typename arg1_type, typename arg2_type, typename arg3_type>
	void attribute(const char* name, const arg1_type& arg1, const arg2_type& arg2, const arg3_type& arg3) const;
	
	template<typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type>
	void attribute(const char* name, const arg1_type& arg1, const arg2_type& arg2, const arg3_type& arg3, const arg4_type& arg4) const;
	
	// functions for setting attribute array variables
	void attribute_array(const char* name, const GLuint& buffer, const GLint& size, const GLenum type = GL_FLOAT, const GLboolean normalized = GL_FALSE, const GLsizei stride = 0) {
		((shader_impl*)this)->attribute_array(name, buffer, size, type, normalized, stride);
	}
	
	// misc functions
#define __A2E_DECLARE_GL_TYPE_CHECK(type) case type: return true;
	static bool is_gl_type(const GLenum& type) {
		switch(type) {
			A2E_GL_SHADER_TYPES(__A2E_DECLARE_GL_TYPE_CHECK);
			default: break;
		}
		return false;
	}
	static bool is_gl_sampler_type(const GLenum& type) {
		switch(type) {
			A2E_GL_SHADER_SAMPLER_TYPES(__A2E_DECLARE_GL_TYPE_CHECK);
			default: break;
		}
		return false;
	}
	
	size_t get_attribute_position(const char* name) const {
#if defined(A2E_DEBUG)
		if(shd_obj.programs.size() <= cur_program) {
			log_error("invalid program #%u!", cur_program);
			return 0;
		}
		if(shd_obj.programs[cur_program]->attributes.count(name) == 0) {
			log_error("unknown attribute name \"%s\"!", name);
			return 0;
		}
#endif
		return shd_obj.programs[cur_program]->attributes.find(name)->second.location;
	}
	
	size_t get_uniform_position(const char* name) const {
#if defined(A2E_DEBUG)
		if(shd_obj.programs.size() <= cur_program) {
			log_error("invalid program #%u!", cur_program);
			return 0;
		}
		if(shd_obj.programs[cur_program]->uniforms.count(name) == 0) {
			log_error("unknown uniform name \"%s\"!", name);
			return 0;
		}
#endif
		return shd_obj.programs[cur_program]->uniforms.find(name)->second.location;
	}
	
	size_t get_block_position(const char* name) const {
#if defined(A2E_DEBUG)
		if(shd_obj.programs.size() <= cur_program) {
			log_error("invalid program #%u!", cur_program);
			return 0;
		}
		if(shd_obj.programs[cur_program]->blocks.count(name) == 0) {
			log_error("unknown uniform block name \"%s\"!", name);
			return 0;
		}
#endif
		return shd_obj.programs[cur_program]->blocks.find(name)->second.location;
	}
	
protected:
	const shader_object& shd_obj;
	size_t cur_program;
	string cur_option;
	set<size_t> active_vertex_attribs;
	
};

// functions for setting uniform variables
template<class shader_impl> template<typename arg1_type>
void shader_base<shader_impl>::uniform(const char* name, const arg1_type& arg1) const {
	((shader_impl*)this)->uniform(name, arg1);
}

template<class shader_impl> template<typename arg1_type>
void shader_base<shader_impl>::uniform(const char* name, const arg1_type& arg1, const size_t& count) const {
	((shader_impl*)this)->uniform(name, arg1, count);
}

template<class shader_impl> template<typename arg1_type, typename arg2_type>
void shader_base<shader_impl>::uniform(const char* name, const arg1_type& arg1, const arg2_type& arg2) const {
	((shader_impl*)this)->uniform(name, arg1, arg2);
}

template<class shader_impl> template<typename arg1_type, typename arg2_type, typename arg3_type>
void shader_base<shader_impl>::uniform(const char* name, const arg1_type& arg1, const arg2_type& arg2, const arg3_type& arg3) const {
	((shader_impl*)this)->uniform(name, arg1, arg2, arg3);
}

template<class shader_impl> template<typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type>
void shader_base<shader_impl>::uniform(const char* name, const arg1_type& arg1, const arg2_type& arg2, const arg3_type& arg3, const arg4_type& arg4) const {
	((shader_impl*)this)->uniform(name, arg1, arg2, arg3, arg4);
}

// function for setting a texture
template<class shader_impl> void shader_base<shader_impl>::texture(const char* name, const GLuint& tex, const GLenum texture_type) const {
	((shader_impl*)this)->texture(name, tex, texture_type);
}

template<class shader_impl> void shader_base<shader_impl>::texture(const char* name, const a2e_texture& tex) const {
	((shader_impl*)this)->texture(name, tex);
}

// functions for setting attribute variables
template<class shader_impl> template<typename arg1_type>
void shader_base<shader_impl>::attribute(const char* name, const arg1_type& arg1) const {
	((shader_impl*)this)->attribute(name, arg1);
}

template<class shader_impl> template<typename arg1_type, typename arg2_type>
void shader_base<shader_impl>::attribute(const char* name, const arg1_type& arg1, const arg2_type& arg2) const {
	((shader_impl*)this)->attribute(name, arg1, arg2);
}

template<class shader_impl> template<typename arg1_type, typename arg2_type, typename arg3_type>
void shader_base<shader_impl>::attribute(const char* name, const arg1_type& arg1, const arg2_type& arg2, const arg3_type& arg3) const {
	((shader_impl*)this)->attribute(name, arg1, arg2, arg3);
}

template<class shader_impl> template<typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type>
void shader_base<shader_impl>::attribute(const char* name, const arg1_type& arg1, const arg2_type& arg2, const arg3_type& arg3, const arg4_type& arg4) const {
	((shader_impl*)this)->attribute(name, arg1, arg2, arg3, arg4);
}

#endif
