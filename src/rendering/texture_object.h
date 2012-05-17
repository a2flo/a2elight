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

#ifndef __A2E_TEXTURE_H__
#define __A2E_TEXTURE_H__

#include "global.h"

struct texture_object {
	enum TEXTURE_FILTERING {
		TF_POINT,
		TF_LINEAR,
		TF_BILINEAR,
		TF_TRILINEAR,
		TF_AUTOMATIC,
	};
	
	string filename = "";
	GLenum texture_type = GL_TEXTURE_2D;
	GLuint tex_num = 0;
	GLsizei width = 1;
	GLsizei height = 1;
	GLint internal_format = GL_RGB8;
	GLenum format = GL_RGB;
	bool alpha = false;
	TEXTURE_FILTERING filtering = TF_AUTOMATIC;
	size_t anisotropic = 0;
	GLint wrap_s = GL_REPEAT;
	GLint wrap_t = GL_REPEAT;
	GLint wrap_r = GL_REPEAT;
	GLenum type = GL_UNSIGNED_BYTE;
	float max_value = numeric_limits<float>::min();
	
	texture_object() {}
	
	~texture_object() {
		if(tex_num > 0) {
			glDeleteTextures(1, &tex_num);
		}
	}
	
	GLuint tex() const {
		return tex_num;
	}
	
private:
	// texture copy is forbidden (otherwise we would have two textures "pointing" to the same opengl texture number)
	texture_object(const texture_object& tex) = delete;
	texture_object& operator=(const texture_object& tex) = delete;
	
};

typedef shared_ptr<texture_object> a2e_texture;
#define make_a2e_texture() (make_shared<texture_object>())

#endif
