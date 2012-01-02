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

#ifndef __GFX_H__
#define __GFX_H__

#include "global.h"

#include "core/core.h"
#include "rendering/extensions.h"
#include "core/bbox.h"

/*! @class gfx
 *  @brief graphical functions
 */

// TODO: better location for this?
enum class DRAW_MODE : unsigned int {
	GEOMETRY_PASS			= 1,
	MATERIAL_PASS			= 2,
	GEOMETRY_ALPHA_PASS		= 3,
	MATERIAL_ALPHA_PASS		= 4,
};

class engine;
class shader;
class A2E_API gfx {
public:
	gfx(engine* e);
	~gfx();
	void init();
	
	// TODO: this is only temporary - add the capability to "reload" already initialized shader objects
	void _init_shader();

	struct rect {
		union {
			struct {
				unsigned int x1;
				unsigned int y1;
				unsigned int x2;
				unsigned int y2;
			};
#if 0 // TODO: reintegrate when gcc finally supports this ...
			struct {
				uint2 low;
				uint2 high;
			};
#endif
		};
		
		rect() : x1(0), y1(0), x2(0), y2(0) {}
		rect(const rect& r) : x1(r.x1), y1(r.y1), x2(r.x2), y2(r.y2) {}
		rect(const unsigned int& x1_, const unsigned int& y1_, const unsigned int& x2_, const unsigned int& y2_) : x1(x1_), y1(y1_), x2(x2_), y2(y2_) {}
	};

	enum FADE_TYPE {
		FT_HORIZONTAL,
		FT_VERTICAL,
		FT_DIAGONAL
	};
	
	enum BLEND_MODE {
		BM_DEFAULT,
		BM_ADD,
		BM_PRE_MUL,
		BM_COLOR,
		BM_ALPHA
	};

	// conversions
	void coord_to_pnt(pnt* point, unsigned int x, unsigned int y);
	void coord_to_ipnt(ipnt* point, int x, int y);
	void pnt_to_rect(gfx::rect* rectangle, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
	void ipnt_to_rect(gfx::rect* rectangle, int x1, int y1, int x2, int y2);
	pnt* coord_to_pnt(unsigned int x, unsigned int y);
	ipnt* coord_to_ipnt(int x, int y);
	gfx::rect* pnt_to_rect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

	// drawing
	void draw_point(pnt* point, unsigned int color);
	void draw_line(pnt* point1, pnt* point2, unsigned int color);
	void draw_line(unsigned int x1, unsigned int y1,
				   unsigned int x2, unsigned int y2,
				   unsigned int color);
	void draw_3d_line(const float3& v1, const float3& v2, unsigned int color);
	
	void draw_rectangle(gfx::rect* rectangle, unsigned int color);
	void draw_rectangle(pnt* p1, pnt* p2, unsigned int color);
	void draw_2colored_rectangle(gfx::rect* rectangle, unsigned int color1, unsigned int color2);
	void draw_filled_rectangle(gfx::rect* rectangle, unsigned int color);
	void draw_fade_rectangle(gfx::rect* rectangle, unsigned int color1, unsigned int color2, FADE_TYPE ft);
	void draw_textured_rectangle(const gfx::rect& rectangle,
								 const coord& bottom_left, const coord& top_right,
								 GLuint texture);
	void draw_textured_color_rectangle(const gfx::rect& rectangle,
									   const coord& bottom_left, const coord& top_right,
									   const float4& color,
									   GLuint texture);
	void draw_textured_color_rectangle(const gfx::rect& rectangle,
									   const coord& bottom_left, const coord& top_right,
									   const float4& mul_color,
									   const float4& add_color,
									   GLuint texture);
	void draw_textured_depth_rectangle(const gfx::rect& rectangle,
									   const coord& bottom_left, const coord& top_right,
									   const float& depth,
									   GLuint texture);
	void draw_textured_depth_color_rectangle(const gfx::rect& rectangle,
											 const coord& bottom_left, const coord& top_right,
											 const float& depth,
											 const float4& color,
											 GLuint texture);
	void draw_textured_depth_color_rectangle(const gfx::rect& rectangle,
											 const coord& bottom_left, const coord& top_right,
											 const float& depth,
											 const float4& mul_color,
											 const float4& add_color,
											 GLuint texture);
	
	void draw_bbox(extbbox* bbox, unsigned int color);
	void draw_fullscreen_triangle() const;
	void draw_textured_fullscreen_triangle() const;
	void draw_fullscreen_quad() const;
	void draw_primitives(void* data, const size_t& size,
						 const GLsizei& vertex_size, const GLenum vertex_type,
						 const GLenum primitive_type, const GLsizei& count);

	// testing
	bool is_pnt_in_rectangle(gfx::rect* rectangle, pnt* point);
	bool is_pnt_in_rectangle(gfx::rect* rectangle, unsigned int x, unsigned int y);
	bool is_pnt_in_rectangle(gfx::rect* rectangle, ipnt* point);
	bool is_pnt_in_rectangle(gfx::rect* rectangle, int x, int y);

	// color funcs
	unsigned int get_color(unsigned int red, unsigned int green, unsigned int blue);
	unsigned int get_color(unsigned int rgb);
	unsigned int get_average_color(unsigned int color1, unsigned int color2);

	// scissor test
	void begin_scissor();
	void set_scissor(gfx::rect* rectangle);
	void set_scissor(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
	void end_scissor();
	
	void set_blend_mode(BLEND_MODE mode);
	
	//
	GLuint get_fullscreen_triangle_vbo() const;
	GLuint get_fullscreen_quad_vbo() const;

protected:
	engine* e;
	ext* exts;
	shader* shd;
	
	pnt points[4];
	float3 verts[4];
	coord rect_coords[4];
	
	GLuint vbo_fullscreen_triangle;
	float fullscreen_triangle[6];
	GLuint vbo_fullscreen_quad;
	float fullscreen_quad[8];
	
	GLuint vbo_primitive;
	GLuint vbo_colors;
	GLuint vbo_coords;
	GLuint vao_primitive;
	void primitive_draw(void* data, const size_t& size,
						const GLsizei& vertex_size, const GLenum vertex_type,
						const float4& color,
						const GLenum primitive_type, const GLsizei& count);
	void primitive_draw_colored(void* data, const size_t& size,
								const GLsizei& vertex_size, const GLenum vertex_type,
								const float4* colors, const size_t& color_count,
								const GLenum primitive_type, const GLsizei& count);
	
	void textured_depth_color_rectangle(const gfx::rect& rectangle,
										const coord& bottom_left, const coord& top_right,
										const float& depth, const bool use_depth,
										const float4& mul_color,
										const bool use_mul_color,
										const float4& add_color,
										const bool use_add_color,
										GLuint texture);
};

#endif
