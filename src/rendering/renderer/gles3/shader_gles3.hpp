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

#ifndef __A2E_SHADER_GLES3_HPP__
#define __A2E_SHADER_GLES3_HPP__

#if defined(FLOOR_IOS) && defined(PLATFORM_X64)

#include "global.hpp"
#include "core/matrix4.hpp"
#include "engine.hpp"
#include "rendering/extensions.hpp"
#include "rendering/rtt.hpp"
#include "rendering/renderer/shader_base.hpp"

class A2E_API shader_gles3 : public shader_base<shader_gles3> {
public:
	shader_gles3(const shader_object& shd_obj_);
	
	virtual void use();
	virtual void use(const size_t& program);
	virtual void use(const string& option, const set<string> combiners = set<string> {});
	virtual void disable();
	virtual size_t get_cur_program() const;
	virtual const string& get_cur_option() const;
	
	// -> uniform
	// 1{i,ui,f,b,fv,iv,uiv,bv}
	void uniform(const char* name, const float& arg1) const;
	void uniform(const char* name, const int& arg1) const;
	void uniform(const char* name, const unsigned int& arg1) const;
	void uniform(const char* name, const bool& arg1) const;
	void uniform(const char* name, const float* arg1, const size_t& count) const;
	void uniform(const char* name, const int* arg1, const size_t& count) const;
	void uniform(const char* name, const unsigned int* arg1, const size_t& count) const;
	void uniform(const char* name, const bool* arg1, const size_t& count) const;
	
	// 2{i,ui,f,b,fv,iv,uiv,bv}
	void uniform(const char* name, const float& arg1, const float& arg2) const;
	void uniform(const char* name, const int& arg1, const int& arg2) const;
	void uniform(const char* name, const unsigned int& arg1, const unsigned int& arg2) const;
	void uniform(const char* name, const bool& arg1, const bool& arg2) const;
	void uniform(const char* name, const float2& arg1) const;
	void uniform(const char* name, const int2& arg1) const;
	void uniform(const char* name, const uint2& arg1) const;
	void uniform(const char* name, const bool2& arg1) const;
	void uniform(const char* name, const float2* arg1, const size_t& count) const;
	void uniform(const char* name, const int2* arg1, const size_t& count) const;
	void uniform(const char* name, const uint2* arg1, const size_t& count) const;
	void uniform(const char* name, const bool2* arg1, const size_t& count) const;
	
	// 3{i,ui,f,b,fv,iv,uiv,bv}
	void uniform(const char* name, const float& arg1, const float& arg2, const float& arg3) const;
	void uniform(const char* name, const int& arg1, const int& arg2, const int& arg3) const;
	void uniform(const char* name, const unsigned int& arg1, const unsigned int& arg2, const unsigned int& arg3) const;
	void uniform(const char* name, const bool& arg1, const bool& arg2, const bool& arg3) const;
	void uniform(const char* name, const float3& arg1) const;
	void uniform(const char* name, const int3& arg1) const;
	void uniform(const char* name, const uint3& arg1) const;
	void uniform(const char* name, const bool3& arg1) const;
	void uniform(const char* name, const float3* arg1, const size_t& count) const;
	void uniform(const char* name, const int3* arg1, const size_t& count) const;
	void uniform(const char* name, const uint3* arg1, const size_t& count) const;
	void uniform(const char* name, const bool3* arg1, const size_t& count) const;
	
	// 4{i,ui,f,b,fv,iv,uiv,bv}
	void uniform(const char* name, const float& arg1, const float& arg2, const float& arg3, const float& arg4) const;
	void uniform(const char* name, const int& arg1, const int& arg2, const int& arg3, const int& arg4) const;
	void uniform(const char* name, const unsigned int& arg1, const unsigned int& arg2, const unsigned int& arg3, const unsigned int& arg4) const;
	void uniform(const char* name, const bool& arg1, const bool& arg2, const bool& arg3, const bool& arg4) const;
	void uniform(const char* name, const float4& arg1) const;
	void uniform(const char* name, const int4& arg1) const;
	void uniform(const char* name, const uint4& arg1) const;
	void uniform(const char* name, const bool4& arg1) const;
	void uniform(const char* name, const float4* arg1, const size_t& count) const;
	void uniform(const char* name, const int4* arg1, const size_t& count) const;
	void uniform(const char* name, const uint4* arg1, const size_t& count) const;
	void uniform(const char* name, const bool4* arg1, const size_t& count) const;
	
	// mat*
	void uniform(const char* name, const matrix4f& arg1) const;
	void uniform(const char* name, const matrix4f* arg1, const size_t& count) const;
	
	// -> texture
	void texture(const char* name, const GLuint& tex, const GLenum texture_type = GL_TEXTURE_2D) const;
	void texture(const char* name, const a2e_texture& tex) const;
	
	// -> attribute
	// 1{f,fv}
	void attribute(const char* name, const float& arg1) const;
	void attribute(const char* name, const float* arg1) const;
	
	// 2{f,fv}
	void attribute(const char* name, const float& arg1, const float& arg2) const;
	void attribute(const char* name, const float2* arg1) const;
	
	// 3{f,fv}
	void attribute(const char* name, const float& arg1, const float& arg2, const float& arg3) const;
	void attribute(const char* name, const float3* arg1) const;
	
	// 4{f,fv}
	void attribute(const char* name, const float& arg1, const float& arg2, const float& arg3, const float& arg4) const;
	void attribute(const char* name, const float4* arg1) const;
	
	// -> attribute array
	void attribute_array(const char* name, const GLuint& buffer, const GLint& size, const GLenum type = GL_FLOAT, const GLboolean normalized = GL_FALSE, const GLsizei stride = 0);
	
	// -> uniform block
	void block(const char* name, const GLuint& ubo) const;
	
protected:
	void set_texture(const char* name, const GLuint& tex, const GLenum& texture_type) const;
	
};

typedef shared_ptr<shader_gles3> gl_shader;
typedef shader_gles3 shader_class;

#endif

#endif
