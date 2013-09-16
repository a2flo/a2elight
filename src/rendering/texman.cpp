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

#include "texman.h"

#if !defined(GL_BGRA8)
#define GL_BGRA8 GL_BGRA
#endif
#if !defined(GL_BGR8)
#define GL_BGR8 GL_BGR
#endif

// ..., internal format, format, type, bpp, red shift, green shift, blue shift, alpha shift
#if !defined(A2E_IOS)

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

#else

// these two are necessary to correctly convert bgr textures to rgb
#define A2E_IOS_GL_BGR 1
#define A2E_IOS_GL_BGR8 2

#define __TEXTURE_FORMATS(F, src_surface, dst_internal_format, dst_format, dst_type) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_R8, GL_RED, GL_UNSIGNED_BYTE, 8, 0, 0, 0, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, 16, 0, 8, 0, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 24, 0, 8, 16, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RGB8, GL_RGBA, GL_UNSIGNED_BYTE, 32, 0, 8, 16, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 32, 0, 8, 16, 24) \
F(src_surface, dst_internal_format, dst_format, dst_type, A2E_IOS_GL_BGR8, A2E_IOS_GL_BGR, GL_UNSIGNED_BYTE, 24, 16, 8, 0, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, A2E_IOS_GL_BGR8, GL_BGRA, GL_UNSIGNED_BYTE, 32, 16, 8, 0, 0) \
F(src_surface, dst_internal_format, dst_format, dst_type, GL_BGRA8, GL_BGRA, GL_UNSIGNED_BYTE, 32, 16, 8, 0, 24)

#endif

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
	texman::f = f_;
	texman::u = u_;
	texman::exts = exts_;
	
	dummy_texture = add_texture(string(datapath+"none.png").c_str(), TEXTURE_FILTERING::POINT, standard_anisotropic, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	standard_filtering = TEXTURE_FILTERING::POINT;
	texman::standard_anisotropic = standard_anisotropic_;
}

/*! deletes the texman object
 */
texman::~texman() {
	textures.clear();
}

a2e_texture texman::check_texture(const string& filename, GLsizei width, GLsizei height, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type) {
	if(filename == "") return dummy_texture;
	
	// check if we already loaded this texture
	for(const auto& tex : textures) {
		if(tex->filename == filename &&
		   tex->height == height &&
		   tex->width == width &&
		   tex->internal_format == internal_format &&
		   tex->format == format &&
		   tex->filtering == filtering &&
		   tex->anisotropic == anisotropic &&
		   tex->wrap_s == wrap_s &&
		   tex->wrap_t == wrap_t &&
		   tex->type == type) {
			// we already loaded the texture, so just return its number
			return tex;
		}
	}
	
	return dummy_texture;
}

a2e_texture texman::add_texture(const string& filename, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t) {
	// create a sdl surface and load the texture
	SDL_Surface* tex_surface = IMG_Load(filename.c_str());
	if(tex_surface == nullptr) {
		log_error("error loading texture file \"%s\" - %s!", filename, SDL_GetError());
		return dummy_texture;
	}
	
	// these values will be computed
	GLint internal_format = 0;
	GLenum format = 0;
	GLenum type = 0;
	
	// figure out the textures format
	check_format(tex_surface, internal_format, format, type);
	
	// if the format is BGR(A), convert it to RGB(A)
#if !defined(A2E_IOS)
	if(format == GL_BGR || format == GL_BGRA) {
#else
	if(format == A2E_IOS_GL_BGR || format == GL_BGRA) {
#endif
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
		if(new_surface == nullptr) {
			log_error("BGR(A)->RGB(A) surface conversion failed!");
		}
		else {
			SDL_FreeSurface(tex_surface);
			tex_surface = new_surface;
		}
	}
		
	// check if this texture already exists
	a2e_texture check_tex = check_texture(filename, tex_surface->w, tex_surface->h, internal_format, format, filtering, anisotropic, wrap_s, wrap_t, type);
	if(check_tex != dummy_texture) return check_tex;
	
	// add
	a2e_texture ret_tex = make_a2e_texture();
	ret_tex->filename = filename;
	ret_tex = add_texture(tex_surface->pixels, tex_surface->w, tex_surface->h, internal_format, format, filtering, anisotropic, wrap_s, wrap_t, type, ret_tex);

	// delete the sdl surface, b/c it isn't needed any more
	SDL_FreeSurface(tex_surface);

	return textures.back();
}

a2e_texture texman::add_texture(const string& filename, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type) {
	// create a sdl surface and load the texture
	SDL_Surface* tex_surface = IMG_Load(filename.c_str());
	if(tex_surface == nullptr) {
		log_error("error loading texture file \"%s\" - %s!", filename, SDL_GetError());
		return dummy_texture;
	}
	
	// check if this texture already exists
	a2e_texture check_tex = check_texture(filename, tex_surface->w, tex_surface->h, internal_format, format, filtering, anisotropic, wrap_s, wrap_t, type);
	if(check_tex != dummy_texture) return check_tex;
	
	// add
	a2e_texture ret_tex = make_a2e_texture();
	ret_tex->filename = filename;
	ret_tex = add_texture(tex_surface->pixels, tex_surface->w, tex_surface->h, internal_format, format, filtering, anisotropic, wrap_s, wrap_t, type, ret_tex);
	
	// delete the sdl surface, b/c it isn't needed any more
	SDL_FreeSurface(tex_surface);
	
	return textures.back();
}

	
a2e_texture texman::add_texture(void* pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type) {
	a2e_texture ret_tex = make_a2e_texture();
	add_texture(pixel_data, width, height, internal_format, format, filtering, anisotropic, wrap_s, wrap_t, type, ret_tex);
	return ret_tex;
}

a2e_texture texman::add_texture(void* pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLenum type, a2e_texture& tex) {
	tex->texture_type = GL_TEXTURE_2D;
	tex->width = width;
	tex->height = height;
	tex->internal_format = internal_format;
	tex->format = format;
	tex->filtering = filtering;
	tex->anisotropic = anisotropic;
	tex->wrap_s = wrap_s;
	tex->wrap_t = wrap_t;
	tex->type = type;
	
	// if "automatic filtering" is specified, use standard filtering (as set in config.xml)
	if(filtering == TEXTURE_FILTERING::AUTOMATIC) filtering = standard_filtering;
	
	// now create/generate an opengl texture and bind it
	glGenTextures(1, &tex->tex_num);
	glBindTexture(GL_TEXTURE_2D, tex->tex_num);
	
	// texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (filtering == TEXTURE_FILTERING::POINT ? GL_NEAREST : GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, select_filter(filtering));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
	
	/*const size_t min_size(std::min(width, height));
	size_t max_level = 0;
	for(size_t size = min_size; size > 2; max_level++) {
		size >>= 1;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_level);*/
	
	if(anisotropic > 0 && filtering >= TEXTURE_FILTERING::BILINEAR) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLint)anisotropic);
	}
	
	// create texture
	glTexImage2D(GL_TEXTURE_2D, 0, convert_internal_format(tex->internal_format), tex->width, tex->height, 0, tex->format, tex->type, pixel_data);
	
	if(filtering > TEXTURE_FILTERING::LINEAR) {
		// build mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
		// TODO: since glGenerateMipmap doesn't seem to work reliably, generate mipmaps ourselves
	}
	
	tex->alpha = get_alpha(tex->format);
	
	// add to textures container
	textures.push_back(tex);
	
	return textures.back();
}

a2e_texture texman::add_cubemap_texture(void** pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLint wrap_r, GLenum type) {
	a2e_texture ret_tex = make_a2e_texture();
	add_cubemap_texture(pixel_data, width, height, internal_format, format, filtering, anisotropic, wrap_s, wrap_t, wrap_r, type, ret_tex);
	return ret_tex;
}

a2e_texture texman::add_cubemap_texture(void** pixel_data, GLsizei width, GLsizei height, GLint internal_format, GLenum format, TEXTURE_FILTERING filtering, size_t anisotropic, GLint wrap_s, GLint wrap_t, GLint wrap_r, GLenum type, a2e_texture& tex) {
	// check if width and height are equal
	if(width != height) {
		log_error("cubemap width and height must be equal!");
		return dummy_texture;
	}
	
	tex->texture_type = GL_TEXTURE_CUBE_MAP;
	tex->width = width;
	tex->height = height;
	tex->internal_format = internal_format;
	tex->format = format;
	tex->filtering = filtering;
	tex->anisotropic = anisotropic;
	tex->wrap_s = wrap_s;
	tex->wrap_t = wrap_t;
	tex->wrap_r = wrap_r;
	tex->type = type;
	
	// if "automatic filtering" is specified, use standard filtering (as set in config.xml)
	if(filtering == TEXTURE_FILTERING::AUTOMATIC) filtering = standard_filtering;
	
	// now create/generate an opengl texture and bind it
	glGenTextures(1, &tex->tex_num);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex->tex_num);
	
	// texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,
					(filtering == TEXTURE_FILTERING::POINT ? GL_NEAREST : GL_LINEAR));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, select_filter(filtering));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_t);
#if !defined(A2E_IOS)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r); // TODO: why isn't this supported on iOS?
#endif
	
	if(anisotropic > 0 && filtering >= TEXTURE_FILTERING::BILINEAR) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLint)anisotropic);
	}
	
	static const GLenum cmap[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
									GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };
	
	for(size_t i = 0; i < 6; i++) {
		glTexImage2D(cmap[i], 0, convert_internal_format(tex->internal_format), tex->width, tex->height, 0, tex->format, tex->type, pixel_data[i]);
		
	}
	if(filtering > TEXTURE_FILTERING::LINEAR) {
		// build mipmaps
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	
	tex->alpha = get_alpha(tex->format);
	
	// add to textures container
	textures.push_back(tex);
	
	return textures.back();
}

void texman::delete_texture(a2e_texture& tex) {
	const auto del_iter = find(textures.begin(), textures.end(), tex);
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
	for(const auto& tex : textures) {
		if(tex->tex_num == tex_num) {
			return tex;
		}
	}
	
	log_error("couldn't find texture #%u", tex_num);
	return dummy_texture;
}

void texman::set_filtering(TEXTURE_FILTERING filtering) {
	if(filtering > TEXTURE_FILTERING::TRILINEAR) {
		log_error("unknown texture filtering mode (%u)!", filtering);
		texman::standard_filtering = TEXTURE_FILTERING::POINT;
		return;
	}
	texman::standard_filtering = filtering;
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
		case GL_RGB8:
#if !defined(A2E_IOS)
		case GL_BGR:
		case GL_R3_G3_B2:
		case GL_RGB4:
		case GL_RGB5:
		case GL_RGB10:
		case GL_RGB12:
		case GL_RGB16:
		case GL_SRGB:
		case GL_SRGB8:
#endif
			ret = 3;
			break;
		case GL_RGBA:
		case GL_BGRA:
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_RGBA8:
		case GL_RGBA16F:
#if !defined(A2E_IOS)
		case GL_RGBA2:
		case GL_RGB10_A2:
		case GL_RGBA12:
		case GL_RGBA16:
		case GL_RGBA32F:
		case GL_SRGB_ALPHA:
		case GL_SRGB8_ALPHA8:
#endif
			ret = 4;
			break;
		default:
			log_error("unknown texture format (%u)!", format);
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
		case GL_RGB8:
#if !defined(A2E_IOS)
		case GL_BGR:
		case GL_R3_G3_B2:
		case GL_RGB4:
		case GL_RGB5:
		case GL_RGB10:
		case GL_RGB12:
		case GL_RGB16:
		case GL_SRGB:
		case GL_SRGB8:
#endif
			ret = false;
			break;
		case GL_RGBA:
		case GL_BGRA:
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_RGBA8:
		case GL_RGBA16F:
#if !defined(A2E_IOS)
		case GL_RGBA2:
		case GL_RGB10_A2:
		case GL_RGBA12:
		case GL_RGBA16:
		case GL_RGBA32F:
		case GL_SRGB_ALPHA:
		case GL_SRGB8_ALPHA8:
#endif
			ret = true;
			break;
		default:
			log_error("unknown texture format (%u)!", format);
			ret = 1;
			break;
	}
	return ret;
}

GLint texman::convert_internal_format(const GLint& internal_format) {
#if !defined(A2E_IOS)
	return internal_format;
#else
	switch(internal_format) {
		case GL_RGB8: return GL_RGB;
		case GL_RGBA4: return GL_RGBA;
		case GL_RGB5_A1: return GL_RGBA;
		case GL_RGBA8: return GL_RGBA;
		case GL_RGBA16F: return GL_RGBA;
		case GL_DEPTH_COMPONENT16: return GL_DEPTH_COMPONENT;
		case GL_DEPTH_COMPONENT24: return GL_DEPTH_COMPONENT;
		
		case GL_RED:
		case GL_RG:
		case GL_RGB:
		case GL_RGBA:
		case GL_BGRA:
		case GL_ALPHA:
		case GL_LUMINANCE:
		case GL_LUMINANCE_ALPHA:
		default: break;
	}
	return internal_format;
#endif
}

GLenum texman::select_filter(const TEXTURE_FILTERING& filter) {
	switch(filter) {
		case TEXTURE_FILTERING::POINT: return GL_NEAREST;
		case TEXTURE_FILTERING::LINEAR: return GL_LINEAR;
		case TEXTURE_FILTERING::BILINEAR: return GL_LINEAR_MIPMAP_NEAREST;
		case TEXTURE_FILTERING::TRILINEAR: return GL_LINEAR_MIPMAP_LINEAR;
		default: break;
	}
	return GL_NEAREST;
}
