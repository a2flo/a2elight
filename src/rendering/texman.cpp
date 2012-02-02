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

#include "texman.h"

#ifndef GL_BGRA8
#define GL_BGRA8 GL_BGRA
#endif
#ifndef GL_BGR8
#define GL_BGR8 GL_BGR
#endif

// ..., internal format, format, type, bpp, red shift, green shift, blue shift, alpha shift
#define __TEXTURE_FORMATS(F, src_surface, dst_internal_format, dst_format, dst_type) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_R8, GL_RED, GL_UNSIGNED_BYTE, 8, 0, 0, 0, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, 16, 0, 8, 0, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 24, 0, 8, 16, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RGB8, GL_RGBA, GL_UNSIGNED_BYTE, 32, 0, 8, 16, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 32, 0, 8, 16, 24) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_BGR8, GL_BGR, GL_UNSIGNED_BYTE, 24, 16, 8, 0, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_BGR8, GL_BGRA, GL_UNSIGNED_BYTE, 32, 16, 8, 0, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_BGRA8, GL_BGRA, GL_UNSIGNED_BYTE, 32, 16, 8, 0, 24) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_R16, GL_RED, GL_UNSIGNED_SHORT, 16, 0, 0, 0, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RG16, GL_RG, GL_UNSIGNED_SHORT, 32, 0, 16, 0, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RGB16, GL_RGB, GL_UNSIGNED_SHORT, 48, 0, 16, 32, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT, 64, 0, 16, 32, 48)
// TODO: float?

#define __CHECK_FORMAT(src_surface, dst_internal_format, dst_format, dst_type, \
					   gl_internal_format, gl_format, gl_type, bpp, rshift, gshift, bshift, ashift) \
if(src_surface->format->Rshift == rshift && \
   src_surface->format->Gshift == gshift && \
   src_surface->format->Bshift == bshift && \
   src_surface->format->Ashift == ashift && \
   src_surface->format->BitsPerPixel == bpp) { \
	dst_internal_format = gl_internal_format; \
	dst_format = gl_format; \
	dst_type = gl_type; \
}

#define check_format(surface, internal_format, format, texture_type) { \
	__TEXTURE_FORMATS(__CHECK_FORMAT, surface, internal_format, format, texture_type); \
}

/*! creates the texman object
 */
texman::texman(file_io* f_, unicode* u_, ext* exts_, const string& datapath, const size_t& standard_anisotropic_) {
	filter[0] = GL_NEAREST;
	filter[1] = GL_LINEAR;
	filter[2] = GL_LINEAR_MIPMAP_NEAREST;
	filter[3] = GL_LINEAR_MIPMAP_LINEAR;

	texman::f = f_;
	texman::u = u_;
	texman::exts = exts_;
	
	dummy_texture = add_texture(string(datapath+"none.png").c_str(), texture_object::TF_POINT, standard_anisotropic, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	standard_filtering = texture_object::TF_POINT;
	texman::standard_anisotropic = standard_anisotropic_;
}

/*! deletes the texman object
 */
texman::~texman() {
	textures.clear();
}

a2e_texture texman::check_texture(const string& filename, GLsizei width, GLsizei height, GLint internal_format, GLenum format, texture_object::TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type) {
	if(filename == "") return dummy_texture;
	
	// check if we already loaded this texture
	for(deque<a2e_texture>::iterator tex_iter = textures.begin(); tex_iter != textures.end(); tex_iter++) {
		if((*tex_iter)->filename == filename &&
		   (*tex_iter)->height == height &&
		   (*tex_iter)->width == width &&
		   (*tex_iter)->internal_format == internal_format &&
		   (*tex_iter)->format == format &&
		   (*tex_iter)->filtering == filtering &&
		   (*tex_iter)->anisotropic == anisotropic &&
		   (*tex_iter)->wrap_s == wrap_s &&
		   (*tex_iter)->wrap_t == wrap_t &&
		   (*tex_iter)->type == type) {
			// we already loaded the texture, so just return its number
			return *tex_iter;
		}
	}
	
	return dummy_texture;
}

a2e_texture texman::add_texture(const string& filename, texture_object::TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t) {
	// create a sdl surface and load the texture
	SDL_Surface* tex_surface = IMG_Load(filename.c_str());
	if(tex_surface == NULL) {
		a2e_error("error loading texture file \"%s\" - %s!", filename, SDL_GetError());
		return dummy_texture;
	}
	
	// these values will be computed
	GLint internal_format = 0;
	GLenum format = 0;
	GLenum type = 0;
	
	// figure out the textures format
	check_format(tex_surface, internal_format, format, type);
	
	// if the format is BGR(A), convert it to RGB(A)
	if(format == GL_BGR || format == GL_BGRA) {
		SDL_PixelFormat new_pformat;
		memcpy(&new_pformat, tex_surface->format, sizeof(SDL_PixelFormat));
		new_pformat.Rshift = tex_surface->format->Bshift;
		new_pformat.Bshift = tex_surface->format->Rshift;
		new_pformat.Rmask = tex_surface->format->Bmask;
		new_pformat.Bmask = tex_surface->format->Rmask;
		
		bool alpha_component = (tex_surface->format->Ashift != 0);
		if(!alpha_component) {
			new_pformat.BytesPerPixel = 3;
			new_pformat.BitsPerPixel = 24;
			format = GL_RGB;
			internal_format = GL_RGB8;
		}
		else if(format == GL_BGRA) {
			new_pformat.BytesPerPixel = 4;
			new_pformat.BitsPerPixel = 32;
			format = GL_RGBA;
			internal_format = GL_RGBA8;
		}
		
		SDL_Surface* new_surface = SDL_ConvertSurface(tex_surface, &new_pformat, 0);
		if(new_surface == NULL) {
			a2e_error("BGR(A)->RGB(A) surface conversion failed!");
		}
		else {
			SDL_FreeSurface(tex_surface);
			tex_surface = new_surface;
		}
	}
	
	a2e_texture ret_tex(new texture_object());
	ret_tex->filename = filename;
	ret_tex = add_texture(tex_surface->pixels, tex_surface->w, tex_surface->h, internal_format, format, filtering, anisotropic, wrap_s, wrap_t, type, &ret_tex);

	// delete the sdl surface, b/c it isn't needed any more
	SDL_FreeSurface(tex_surface);

	return textures.back();
}

a2e_texture texman::add_texture(const string& filename, GLint internal_format, GLenum format, texture_object::TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type) {
	// create a sdl surface and load the texture
	SDL_Surface* tex_surface = IMG_Load(filename.c_str());
	if(tex_surface == NULL) {
		a2e_error("error loading texture file \"%s\" - %s!", filename, SDL_GetError());
		return dummy_texture;
	}
	
	// check if this texture already exists
	a2e_texture check_tex = check_texture(filename, tex_surface->w, tex_surface->h, internal_format, format, filtering, anisotropic, wrap_s, wrap_t, type);
	if(check_tex != dummy_texture) return check_tex;
	
	a2e_texture ret_tex(new texture_object());
	ret_tex->filename = filename;
	ret_tex = add_texture(tex_surface->pixels, tex_surface->w, tex_surface->h, internal_format, format, filtering, anisotropic, wrap_s, wrap_t, type, &ret_tex);
	
	// delete the sdl surface, b/c it isn't needed any more
	SDL_FreeSurface(tex_surface);
	
	return textures.back();
}

a2e_texture texman::add_texture(void* pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, texture_object::TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type, a2e_texture* tex) {
	a2e_texture ret_tex(tex == NULL ? make_a2e_texture() : *tex);
	
	ret_tex->texture_type = GL_TEXTURE_2D;
	ret_tex->width = width;
	ret_tex->height = height;
	ret_tex->internal_format = internal_format;
	ret_tex->format = format;
	ret_tex->filtering = filtering;
	ret_tex->anisotropic = anisotropic;
	ret_tex->wrap_s = wrap_s;
	ret_tex->wrap_t = wrap_t;
	ret_tex->type = type;
	
	// if "automatic filtering" is specified, use standard filtering (as set in config.xml)
	if(filtering == texture_object::TF_AUTOMATIC) filtering = standard_filtering;
	
	// now create/generate an opengl texture and bind it
	glGenTextures(1, &ret_tex->tex_num);
	glBindTexture(GL_TEXTURE_2D, ret_tex->tex_num);
	
	// texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (filtering == 0 ? GL_NEAREST : GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter[filtering]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
	
	if(anisotropic > 0 && filtering >= texture_object::TF_BILINEAR) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLint)anisotropic);
	}
	
	// create texture
	glTexImage2D(GL_TEXTURE_2D, 0, ret_tex->internal_format, ret_tex->width, ret_tex->height, 0, ret_tex->format, ret_tex->type, pixel_data);
	
	if(filtering > 1) {
		// build mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	
	ret_tex->alpha = get_alpha(ret_tex->format);
	
	// add to textures container
	textures.push_back(ret_tex);
	
	return textures.back();
}

a2e_texture texman::add_cubemap_texture(void** pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, texture_object::TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLint wrap_r, GLenum type, a2e_texture* tex) {
	// check if width and height are equal
	if(width != height) {
		a2e_error("cubemap width and height must be equal!");
		return dummy_texture;
	}
	
	a2e_texture ret_tex(tex == NULL ? make_a2e_texture() : *tex);
	
	ret_tex->texture_type = GL_TEXTURE_CUBE_MAP;
	ret_tex->width = width;
	ret_tex->height = height;
	ret_tex->internal_format = internal_format;
	ret_tex->format = format;
	ret_tex->filtering = filtering;
	ret_tex->anisotropic = anisotropic;
	ret_tex->wrap_s = wrap_s;
	ret_tex->wrap_t = wrap_t;
	ret_tex->wrap_r = wrap_r;
	ret_tex->type = type;
	
	// if "automatic filtering" is specified, use standard filtering (as set in config.xml)
	if(filtering == texture_object::TF_AUTOMATIC) filtering = standard_filtering;
	
	// now create/generate an opengl texture and bind it
	glGenTextures(1, &ret_tex->tex_num);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ret_tex->tex_num);
	
	// texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, (filtering == 0 ? GL_NEAREST : GL_LINEAR));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter[filtering]);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_t);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r);
	
	if(anisotropic > 0 && filtering >= texture_object::TF_BILINEAR) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLint)anisotropic);
	}
	
	static const GLenum cmap[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
									GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };
	
	for(size_t i = 0; i < 6; i++) {
		glTexImage2D(cmap[i], 0, ret_tex->internal_format, ret_tex->width, ret_tex->height, 0, ret_tex->format, ret_tex->type, pixel_data[i]);
		
	}
	if(filtering > 1) {
		// build mipmaps
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	
	ret_tex->alpha = get_alpha(ret_tex->format);
	
	// add to textures container
	textures.push_back(ret_tex);
	
	return textures.back();
}

void texman::delete_texture(a2e_texture& tex) {
	deque<a2e_texture>::iterator del_iter = find(textures.begin(), textures.end(), tex);
	if(del_iter != textures.end()) {
		//delete &(*del_iter);
		textures.erase(del_iter);
	}
	tex = dummy_texture;
}

const a2e_texture texman::get_dummy_texture() const {
	return dummy_texture;
}

/*! returns the texture with the opengl tex number num
 *  @param tex_num the opengl texture number we want to search for
 */
a2e_texture texman::get_texture(GLuint tex_num) {
	for(deque<a2e_texture>::iterator tex_iter = textures.begin(); tex_iter != textures.end(); tex_iter++) {
		if((*tex_iter)->tex_num == tex_num) {
			return *tex_iter;
		}
	}
	
	a2e_error("couldn't find texture #%u", tex_num);
	return dummy_texture;
}

void texman::set_filtering(unsigned int filtering) {
	if(filtering > texture_object::TF_TRILINEAR) {
		a2e_error("unknown texture filtering mode (%u)!", filtering);
		texman::standard_filtering = texture_object::TF_POINT;
		return;
	}
	texman::standard_filtering = (texture_object::TEXTURE_FILTERING)filtering;
}

unsigned int texman::get_components(GLint format) {
	unsigned int ret = 0;
	switch(format) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			ret = format;
			break;
		case GL_ALPHA:
		case GL_RED:
			ret = 1;
			break;
		case GL_RG:
			ret = 2;
			break;
		case GL_RGB:
		case GL_BGR:
		case GL_R3_G3_B2:
		case GL_RGB4:
		case GL_RGB5:
		case GL_RGB8:
		case GL_RGB10:
		case GL_RGB12:
		case GL_RGB16:
		case GL_SRGB:
		case GL_SRGB8:
			ret = 3;
			break;
		case GL_RGBA:
		case GL_BGRA:
		case GL_RGBA2:
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_RGBA8:
		case GL_RGB10_A2:
		case GL_RGBA12:
		case GL_RGBA16:
		case GL_RGBA16F:
		case GL_RGBA32F:
		case GL_SRGB_ALPHA:
		case GL_SRGB8_ALPHA8:
			ret = 4;
			break;
		default:
			a2e_error("unknown texture format (%u)!", format);
			ret = 1;
			break;
	}
	return ret;
}

bool texman::get_alpha(GLint format) {
	bool ret = false;
	switch(format) {
		case 0:
		case 1:
		case 2:
		case 3:
			ret = false;
			break;
		case 4:
			ret = true;
			break;
		case GL_ALPHA:
			ret = true;
			break;
		case GL_RED:
		case GL_RG:
		case GL_RGB:
		case GL_BGR:
		case GL_R3_G3_B2:
		case GL_RGB4:
		case GL_RGB5:
		case GL_RGB8:
		case GL_RGB10:
		case GL_RGB12:
		case GL_RGB16:
		case GL_SRGB:
		case GL_SRGB8:
			ret = false;
			break;
		case GL_RGBA:
		case GL_BGRA:
		case GL_RGBA2:
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_RGBA8:
		case GL_RGB10_A2:
		case GL_RGBA12:
		case GL_RGBA16:
		case GL_RGBA16F:
		case GL_RGBA32F:
		case GL_SRGB_ALPHA:
		case GL_SRGB8_ALPHA8:
			ret = true;
			break;
		default:
			a2e_error("unknown texture format (%u)!", format);
			ret = 1;
			break;
	}
	return ret;
}
