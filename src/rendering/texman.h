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

#ifndef __TEXMAN_H__
#define __TEXMAN_H__

#include "global.h"

#include "core/file_io.h"
#include "core/vector3.h"
#include "rendering/extensions.h"
#include "rendering/texture_object.h"
#include "gui/unicode.h"

/*! @class texman
 *  @brief texture management routines
 */

struct texture;
class A2E_API texman {
public:
	texman(file_io* f, unicode* u, ext* exts, const string& datapath, const size_t& standard_anisotropic);
	~texman();
	
	
	a2e_texture add_texture(const string& filename, texture_object::TEXTURE_FILTERING filtering = texture_object::TF_AUTOMATIC, size_t anisotropic = 0, GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT);
	a2e_texture add_texture(const string& filename, GLint internal_format, GLenum format, texture_object::TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type);
	a2e_texture add_texture(void* pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, texture_object::TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type, a2e_texture* tex = nullptr);
	a2e_texture add_cubemap_texture(void** pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, texture_object::TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLint wrap_r, GLenum type, a2e_texture* tex = nullptr);
	void delete_texture(a2e_texture& tex);

	a2e_texture get_texture(GLuint tex_num);
	
	void set_filtering(unsigned int filtering);

	unsigned int get_components(GLint format);
	bool get_alpha(GLint format);
	
	const a2e_texture get_dummy_texture() const;

protected:
	file_io* f;
	unicode* u;
	ext* exts;

	a2e_texture dummy_texture;
	deque<a2e_texture> textures;
	a2e_texture check_texture(const string& filename, GLsizei width, GLsizei height, GLint internal_format, GLenum format, texture_object::TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type);
	
	unsigned int filter[4];

	texture_object::TEXTURE_FILTERING standard_filtering;
	size_t standard_anisotropic;

};

#endif
