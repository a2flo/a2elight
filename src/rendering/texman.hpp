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

#ifndef __A2E_TEXMAN_HPP__
#define __A2E_TEXMAN_HPP__

#include "global.hpp"

#include "core/file_io.hpp"
#include "math/vector_lib.hpp"
#include "rendering/extensions.hpp"
#include "rendering/texture_object.hpp"
#include "core/unicode.hpp"

/*! @class texman
 *  @brief texture management routines
 */

struct texture;
class A2E_API texman {
public:
	texman(ext* exts, const size_t& standard_anisotropic);
	~texman();
	
	
	a2e_texture add_texture(const string& filename, TEXTURE_FILTERING filtering = TEXTURE_FILTERING::AUTOMATIC, size_t anisotropic = 0, GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT);
	
	a2e_texture add_texture(const string& filename, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type);
	a2e_texture add_texture(void* pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type, a2e_texture& tex);
	a2e_texture add_texture(void* pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type);
	
	a2e_texture add_cubemap_texture(void** pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLint wrap_r, GLenum type, a2e_texture& tex);
	a2e_texture add_cubemap_texture(void** pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLint wrap_r, GLenum type);
	
	void delete_texture(a2e_texture& tex);

	a2e_texture get_texture(GLuint tex_num);
	
	void set_filtering(TEXTURE_FILTERING filtering);

	unsigned int get_components(GLint format);
	bool get_alpha(GLint format);
	
	const a2e_texture get_dummy_texture() const;
	
	static GLint convert_internal_format(const GLint& internal_format);
	static GLenum select_filter(const TEXTURE_FILTERING& filter);

protected:
	ext* exts;

	a2e_texture dummy_texture;
	deque<a2e_texture> textures;
	a2e_texture check_texture(const string& filename, GLsizei width, GLsizei height, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type);

	TEXTURE_FILTERING standard_filtering;
	size_t standard_anisotropic;

};

#endif
