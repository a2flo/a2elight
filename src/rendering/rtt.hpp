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

#ifndef __A2E_RENDER_TO_TEXTURE_HPP__
#define __A2E_RENDER_TO_TEXTURE_HPP__

#include "global.hpp"

#include "core/core.hpp"
#include "rendering/extensions.hpp"
#include "rendering/texture_object.hpp"

/*! @class rtt
 *  @brief render to texture class
 */

class A2E_API rtt {
public:
	rtt(ext* exts);
	~rtt();

	enum class TEXTURE_ANTI_ALIASING {
		NONE,
		MSAA_2,
		MSAA_4,
		MSAA_8,
		MSAA_16,
		MSAA_32,
		MSAA_64,
#if !defined(FLOOR_IOS)
		CSAA_8,		// 4 col/8 cov
		CSAA_8Q,	// 8 col/8 cov
		CSAA_16,	// 4 col/16 cov
		CSAA_16Q,	// 8 col/16 cov
		CSAA_32,	// TODO: ratio?
		CSAA_32Q,	// TODO: ratio?
#endif
		SSAA_2,
		SSAA_4,
		FXAA,
		SSAA_4_3_FXAA,
		SSAA_2_FXAA,
	};
	static const char* TEXTURE_ANTI_ALIASING_STR[];
	unsigned int get_sample_count(const TEXTURE_ANTI_ALIASING& taa) const;
	float get_anti_aliasing_scale(const TEXTURE_ANTI_ALIASING& taa) const;
	float2 get_resolution_for_scale(const float& scale, const size2& res) const;
	
	enum class DEPTH_TYPE {
		NONE,
		RENDERBUFFER,
		TEXTURE_2D
	};
	enum class STENCIL_TYPE {
		NONE,
		STENCIL_8
	};

	struct fbo {
		const unsigned int attachment_count = 0;
		unsigned int fbo_id = 0;
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int draw_width = 0;
		unsigned int draw_height = 0;
		unsigned int color_buffer = 0;
		unsigned int depth_buffer = 0;
		DEPTH_TYPE depth_type = DEPTH_TYPE::NONE;
		STENCIL_TYPE stencil_type = STENCIL_TYPE::NONE;
		GLenum depth_attachment_type = GL_DEPTH_ATTACHMENT;
		unsigned int samples = 0;
		
		vector<unsigned int> tex;
		vector<unsigned int> resolve_buffer;
		vector<bool> auto_mipmap;
		vector<GLenum> target;
		vector<TEXTURE_ANTI_ALIASING> anti_aliasing;

		fbo(const unsigned int& count) : attachment_count(count) {
			tex.resize(count);
			resolve_buffer.resize(count);
			auto_mipmap.resize(count);
			target.resize(count);
			anti_aliasing.resize(count);
			
			tex.assign(count, 0);
			resolve_buffer.assign(count, 0);
			auto_mipmap.assign(count, false);
			target.assign(count, GL_TEXTURE_2D);
			anti_aliasing.assign(count, TEXTURE_ANTI_ALIASING::NONE);
		}
	};

	rtt::fbo* add_buffer(const unsigned int width, const unsigned int height,
						 const GLenum target = GL_TEXTURE_2D,
						 const TEXTURE_FILTERING filtering = TEXTURE_FILTERING::POINT,
						 const TEXTURE_ANTI_ALIASING taa = TEXTURE_ANTI_ALIASING::NONE,
						 const GLint wrap_s = GL_REPEAT, const GLint wrap_t = GL_REPEAT,
						 const GLint internal_format = GL_RGBA8,
						 const GLenum format = GL_RGBA,
						 const GLenum type = GL_UNSIGNED_BYTE,
						 const unsigned int attachment_count = 1,
						 const rtt::DEPTH_TYPE depth_type = DEPTH_TYPE::NONE,
						 const rtt::STENCIL_TYPE stencil_type = STENCIL_TYPE::NONE);
	rtt::fbo* add_buffer(const unsigned int width, const unsigned int height,
						 const GLenum* target, const TEXTURE_FILTERING* filtering,
						 const TEXTURE_ANTI_ALIASING* taa,
						 const GLint* wrap_s, const GLint* wrap_t,
						 const GLint* internal_format,
						 const GLenum* format,
						 const GLenum* type,
						 const unsigned int attachment_count = 1,
						 const rtt::DEPTH_TYPE depth_type = DEPTH_TYPE::NONE,
						 const rtt::STENCIL_TYPE stencil_type = STENCIL_TYPE::NONE);
	void delete_buffer(rtt::fbo* buffer);
	void start_draw(rtt::fbo* buffer);
	void stop_draw();
	void start_2d_draw();
	void stop_2d_draw();
	void clear(const unsigned int and_mask = 0xFFFFFFFF,
			   const float4 clear_color = float4 { 0.0f });
	void check_fbo(rtt::fbo* buffer);
	const fbo* get_current_buffer() const;

	void mipmap();

protected:
	ext* exts;

	vector<fbo*> buffers;
	fbo* current_buffer = nullptr;

};

#endif
