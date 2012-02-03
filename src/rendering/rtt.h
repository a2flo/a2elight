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

#ifndef __RENDER_TO_TEXTURE_H__
#define __RENDER_TO_TEXTURE_H__

#include "global.h"

#include "core/core.h"
#include "rendering/gfx.h"
#include "rendering/extensions.h"
#include "rendering/texture_object.h"

/*! @class rtt
 *  @brief render to texture class
 */

class engine;
class A2E_API rtt {
public:
	rtt(engine* e, gfx* g, ext* exts, unsigned int screen_width, unsigned int screen_height);
	~rtt();

	enum TEXTURE_ANTI_ALIASING {
		TAA_NONE,
		TAA_MSAA_1,
		TAA_MSAA_2,
		TAA_MSAA_4,
		TAA_MSAA_8,
		TAA_MSAA_16,
		TAA_MSAA_32,
		TAA_MSAA_64,
		TAA_CSAA_8,		// 4 col/8 cov
		TAA_CSAA_8Q,	// 8 col/8 cov
		TAA_CSAA_16,	// 4 col/16 cov
		TAA_CSAA_16Q,	// 8 col/16 cov
		TAA_CSAA_32,	// TODO: ratio?
		TAA_CSAA_32Q,	// TODO: ratio?
		TAA_SSAA_2,
		TAA_SSAA_4,
		TAA_FXAA,
		TAA_SSAA_4_3_FXAA,
		TAA_SSAA_2_FXAA,
	};
	static const char* TEXTURE_ANTI_ALIASING_STR[];
	size_t get_sample_count(const TEXTURE_ANTI_ALIASING& taa) const;
	float get_anti_aliasing_scale(const TEXTURE_ANTI_ALIASING& taa) const;
	float2 get_resolution_for_scale(const float& scale, const size2& res) const;
	
	enum DEPTH_TYPE {
		DT_NONE,
		DT_RENDERBUFFER,
		DT_TEXTURE_2D
	};

	struct fbo {
		unsigned int fbo_id;
		unsigned int attachment_count;
		unsigned int* tex_id;
		unsigned int width;
		unsigned int height;
		unsigned int draw_width;
		unsigned int draw_height;
		unsigned int color_buffer;
		unsigned int depth_buffer;
		unsigned int stencil_buffer;
		unsigned int* resolve_buffer;
		bool color;
		DEPTH_TYPE depth_type;
		size_t samples;
		bool stencil;
		bool* auto_mipmap;
		GLenum* target;
		TEXTURE_ANTI_ALIASING* anti_aliasing;

		fbo() : fbo_id(0), attachment_count(0), tex_id(NULL), width(0), height(0), draw_width(0), draw_height(0), color_buffer(0), depth_buffer(0), stencil_buffer(0),
			resolve_buffer(NULL), color(false), depth_type(DT_NONE), samples(0), stencil(false), auto_mipmap(NULL), target(NULL), anti_aliasing(NULL) {}
	};

	rtt::fbo* add_buffer(unsigned int width, unsigned int height, GLenum target = GL_TEXTURE_2D, texture_object::TEXTURE_FILTERING filtering = texture_object::TF_POINT, TEXTURE_ANTI_ALIASING taa = TAA_NONE, GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT, GLint internal_format = GL_RGBA8, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE, unsigned int attachment_count = 1, rtt::DEPTH_TYPE depth_type = rtt::DT_NONE);
	rtt::fbo* add_buffer(unsigned int width, unsigned int height, GLenum* target, texture_object::TEXTURE_FILTERING* filtering, TEXTURE_ANTI_ALIASING* taa, GLint* wrap_s, GLint* wrap_t, GLint* internal_format, GLenum* format, GLenum* type, unsigned int attachment_count = 1, rtt::DEPTH_TYPE depth_type = rtt::DT_NONE);
	void delete_buffer(rtt::fbo* buffer);
	void start_draw(rtt::fbo* buffer);
	void stop_draw();
	void start_2d_draw();
	void stop_2d_draw();
	void clear(const unsigned int and_mask = ~0);
	void check_fbo(rtt::fbo* buffer);
	const fbo* get_current_buffer() const;

	void mipmap();

protected:
	engine* e;
	gfx* g;
	ext* exts;

	vector<fbo*> buffers;
	fbo* current_buffer;

	unsigned int filter[4];

	unsigned int screen_width;
	unsigned int screen_height;

};

#endif
