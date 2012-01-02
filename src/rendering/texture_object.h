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

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "global.h"

struct texture_object {
	enum TEXTURE_FILTERING {
		TF_POINT,
		TF_LINEAR,
		TF_BILINEAR,
		TF_TRILINEAR,
		TF_AUTOMATIC,
	};
	
	string filename;
	GLenum texture_type;
	GLuint tex_num;
	GLsizei width;
	GLsizei height;
	GLint internal_format;
	GLenum format;
	bool alpha;
	TEXTURE_FILTERING filtering;
	size_t anisotropic;
	GLint wrap_s;
	GLint wrap_t;
	GLint wrap_r;
	GLenum type;
	float max_value;
	
	texture_object() : filename(""), texture_type(GL_TEXTURE_2D), tex_num(0), width(1), height(1), internal_format(GL_RGB8), format(GL_RGB), alpha(false), filtering(TF_AUTOMATIC),
		anisotropic(0), wrap_s(GL_REPEAT), wrap_t(GL_REPEAT), wrap_r(GL_REPEAT), type(GL_UNSIGNED_BYTE), max_value(-__FLT_MAX__) {
	}
	
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
	texture_object(const texture_object& tex) {}
	texture_object& operator=(const texture_object& tex) { return *this; }
	
};

typedef shared_ptr<texture_object> a2e_texture;
#define make_a2e_texture() (a2e_texture(new texture_object()))

#endif
