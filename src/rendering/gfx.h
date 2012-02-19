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
		
		void set(const unsigned int& x1_, const unsigned int& y1_, const unsigned int& x2_, const unsigned int& y2_) {
			x1 = x1_; y1 = y1_; x2 = x2_; y2 = y2_;
		}
		
		friend ostream& operator<<(ostream& output, const rect& r) {
			output << "(" << r.x1 << ", " << r.y1 << ") x (" << r.x2 << ", " << r.y2 << ")";
			return output;
		}
		
		rect() : x1(0), y1(0), x2(0), y2(0) {}
		rect(const rect& r) : x1(r.x1), y1(r.y1), x2(r.x2), y2(r.y2) {}
		rect(const unsigned int& x1_, const unsigned int& y1_, const unsigned int& x2_, const unsigned int& y2_) : x1(x1_), y1(y1_), x2(x2_), y2(y2_) {}
	};

	enum class GRADIENT_TYPE {
		HORIZONTAL,
		VERTICAL,
		DIAGONAL
	};
	
	enum class BLEND_MODE {
		DEFAULT,
		ADD,
		PRE_MUL,
		COLOR,
		ALPHA
	};
	
	// drawing
	void draw_point(const pnt& point, const float4& color);
	void draw_line(const pnt& point1, const pnt& point2, const float4& color);
	void draw_line(unsigned int x1, unsigned int y1,
				   unsigned int x2, unsigned int y2,
				   const float4& color);
	void draw_3d_line(const float3& v1, const float3& v2, const float4& color);
	
	void draw_rectangle(const gfx::rect& rectangle, const float4& color);
	void draw_rectangle(const pnt& p1, const pnt& p2, const float4& color);
	void draw_2colored_rectangle(const gfx::rect& rectangle, const float4& color1, const float4& color2);
	void draw_filled_rectangle(const gfx::rect& rectangle, const float4& color);
	// NOTE: corner bools: right top, right bottom, left bottom, left top
	void draw_filled_rounded_rectangle(const gfx::rect& rectangle, const float& radius, const float4& color);
	void draw_filled_rounded_rectangle(const gfx::rect& rectangle, const float& radius, const bool4& corners, const float4& color);
	void draw_gradient_rectangle(const gfx::rect& rectangle, const float4& color1, const float4& color2, const GRADIENT_TYPE gt);
	void draw_textured_rectangle(const gfx::rect& rectangle,
								 const coord& bottom_left, const coord& top_right,
								 GLuint texture);
	void draw_textured_passthrough_rectangle(const gfx::rect& rectangle,
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
	
	void draw_circle(const pnt& p, const float& radius, const float4& color);
	void draw_ellipsoid(const pnt& p, const float& radius_lr, const float& radius_tb, const float4& color);
	void draw_circle_sector(const pnt& p, const float& radius, const float& start_angle, const float& end_angle, const float4& color);
	
	void draw_bbox(const extbbox& bbox, const float4& color);
	void draw_fullscreen_triangle() const;
	void draw_textured_fullscreen_triangle() const;
	void draw_fullscreen_quad() const;
	void draw_primitives(void* data, const size_t& size,
						 const GLsizei& vertex_size, const GLenum vertex_type,
						 const GLenum primitive_type, const GLsizei& count);

	// testing
	static bool is_pnt_in_rectangle(const gfx::rect& rectangle, const pnt& point);
	static bool is_pnt_in_rectangle(const gfx::rect& rectangle, const unsigned int& x, const unsigned int& y);
	static bool is_pnt_in_rectangle(const gfx::rect& rectangle, const ipnt& point);
	static bool is_pnt_in_rectangle(const gfx::rect& rectangle, const int& x, const int& y);

	// scissor test
	static void begin_scissor();
	static void set_scissor(const gfx::rect& rectangle);
	static void set_scissor(const unsigned int& x1, const unsigned int& y1, const unsigned int& x2, const unsigned int& y2);
	static void end_scissor();
	
	static void set_blend_mode(const BLEND_MODE mode);
	
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
										const bool passthrough,
										GLuint texture);
	
	void compute_ellipsoid_points(vector<float2>& dst_points, const float& radius_lr, const float& radius_tb, const float& start_angle, const float& end_angle);
	
};

#endif
