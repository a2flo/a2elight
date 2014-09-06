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

#ifndef __A2E_GFX2D_HPP__
#define __A2E_GFX2D_HPP__

#include "global.hpp"
#include "core/core.hpp"
#include "math/basic_math.hpp"

#include "rendering/shader.hpp"
#include "rendering/extensions.hpp"

/*! @class gfx2d
 *  @brief graphical functions
 */

#define __GFX2D_POINT_COMPUTE_FUNCS(F, DS_FUNC, DS_NAME) \
F(gfx2d::point_compute_point, point, DS_FUNC, DS_NAME) \
F(gfx2d::point_compute_line, line, DS_FUNC, DS_NAME) \
F(gfx2d::point_compute_triangle, triangle, DS_FUNC, DS_NAME) \
F(gfx2d::point_compute_rectangle, rectangle, DS_FUNC, DS_NAME) \
F(gfx2d::point_compute_rounded_rectangle, rounded_rectangle, DS_FUNC, DS_NAME) \
F(gfx2d::point_compute_circle, circle, DS_FUNC, DS_NAME) \
F(gfx2d::point_compute_circle_sector, circle_sector, DS_FUNC, DS_NAME)

#define __GFX2D_DRAW_STYLE_FUNCS(PF, F) \
F(PF, gfx2d::draw_style_fill, fill) \
F(PF, gfx2d::draw_style_gradient, gradient) \
F(PF, gfx2d::draw_style_texture, texture) \
F(PF, gfx2d::draw_style_border<gfx2d::draw_style_fill>, border_fill) \
F(PF, gfx2d::draw_style_border<gfx2d::draw_style_gradient>, border_gradient) \
F(PF, gfx2d::draw_style_border<gfx2d::draw_style_texture>, border_texture)

#define __GFX2D_DEFINE_DRAW_FUNC(pc_func, pc_name, ds_func, ds_name) \
template<typename... Args> static void draw_ ##pc_name ##_ ##ds_name(const Args&... args) { \
	draw<pc_func<ds_func>>(args...); \
}

class A2E_API gfx2d {
public:
	//
	gfx2d() = delete;
	~gfx2d() = delete;
	static void init();
	static void destroy();
	
	//
	enum class GRADIENT_TYPE : unsigned int {
		HORIZONTAL,		//!< left to right
		VERTICAL,		//!< top to bottom
		DIAGONAL_LR,	//!< left bottom to top right
		DIAGONAL_RL,	//!< right bottom to top left
		CENTER,			//!< inside to outside ("linear" -> straight shape)
		CENTER_ROUND,	//!< inside to outside ("quadratic" -> round shape)
	};
	static constexpr const char* gradient_type_to_string(const GRADIENT_TYPE& type) {
		switch(type) {
			case GRADIENT_TYPE::HORIZONTAL: return "gradient_horizontal";
			case GRADIENT_TYPE::VERTICAL: return "gradient_vertical";
			case GRADIENT_TYPE::DIAGONAL_LR: return "gradient_diagonal_lr";
			case GRADIENT_TYPE::DIAGONAL_RL: return "gradient_diagonal_rl";
			case GRADIENT_TYPE::CENTER: return "gradient_center";
			case GRADIENT_TYPE::CENTER_ROUND: return "gradient_center_round";
		}
		floor_unreachable();
	}
	
	enum class BLEND_MODE : unsigned int {
		DEFAULT,
		ADD,
		PRE_MUL,
		COLOR,
		ALPHA
	};
	
	enum class CORNER : unsigned int {
		NONE			= 0,
		TOP_RIGHT		= (1 << 0),
		BOTTOM_RIGHT	= (1 << 1),
		BOTTOM_LEFT		= (1 << 2),
		TOP_LEFT		= (1 << 3),
		ALL				= (TOP_RIGHT | BOTTOM_RIGHT | BOTTOM_LEFT | TOP_LEFT)
	};
	enum_class_bitwise_or(CORNER)
	
	// draw functions
	template <class point_compute_draw_spec, typename... Args>
	static void draw(const Args&... args) {
		point_compute_draw_spec::compute_and_draw(args...);
	}
	
	template <class draw_style> struct point_compute_point;
	template <class draw_style> struct point_compute_line;
	template <class draw_style> struct point_compute_triangle;
	template <class draw_style> struct point_compute_rectangle;
	template <class draw_style> struct point_compute_rounded_rectangle;
	template <class draw_style> struct point_compute_circle;
	template <class draw_style> struct point_compute_circle_sector;
	
	struct draw_style_fill;
	struct draw_style_gradient;
	struct draw_style_texture;
	template <class draw_style_next> struct draw_style_border;
	
	// some macro voodoo for user convenience (e.g. draw_rectangle_gradient(...))
	__GFX2D_DRAW_STYLE_FUNCS(__GFX2D_DEFINE_DRAW_FUNC, __GFX2D_POINT_COMPUTE_FUNCS)
	
	struct primitive_properties {
		vector<float2> points;
		float4 extent;
		GLenum primitive_type;
		union {
			struct {
				// specifies if the primitive has a mid point (e.g. circle, rounded rect)
				unsigned int has_mid_point : 1;
				unsigned int has_equal_start_end_point : 1;
				unsigned int border_connect_start_end_point : 1;
				unsigned int border_swap_strip_points : 1;
				
				//
				unsigned int _unused : 28;
			};
			unsigned int flags = 0;
		};
		primitive_properties(const GLenum& primitive_type_) :
		points(), extent(), primitive_type(primitive_type_) {}
		primitive_properties() :
		points(), extent(), primitive_type(GL_TRIANGLES) {}
		primitive_properties& operator=(primitive_properties&& props) noexcept {
			points.swap(props.points);
			extent = props.extent;
			primitive_type = props.primitive_type;
			flags = props.flags;
			return *this;
		}
	};
	
	// additional draw functions
	static void draw_fullscreen_triangle();
	static void draw_fullscreen_quad();
	
	// helper functions
	static void set_blend_mode(const BLEND_MODE mode);
			
	static bool is_pnt_in_rectangle(const rect& rectangle, const pnt& point);
	static bool is_pnt_in_rectangle(const rect& rectangle, const ipnt& point);
	
	static GLuint get_fullscreen_triangle_vbo();
	static GLuint get_fullscreen_quad_vbo();

	static void compute_ellipsoid_points(vector<float2>& dst_points, const float& radius_lr, const float& radius_tb, const float& start_angle, const float& end_angle);
	
protected:
	static shader* eshd;
	static ext* exts;
	
	static gl_shader simple_shd;
	static gl_shader gradient_shd;
	static gl_shader texture_shd;
			
	static GLuint vbo_primitive;
	
	static void reload_shaders();
	static event::handler evt_handler;
	static bool event_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	
	static void upload_points_and_draw(const gl_shader& shd, const primitive_properties& props);
	
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/*
 *	// point compute interface:
 *  template <class draw_style>
 *	struct point_compute_interface {
 *		template<typename... Args> static void compute_and_draw(...args...);
 *	};
 */

template <class draw_style>
struct gfx2d::point_compute_point {
	template<typename... Args> static void compute_and_draw(const float2& p,
															const Args&... args) {
		primitive_properties props(GL_TRIANGLE_STRIP);
		props.points = {
			float2(p.x, p.y + 1.0f),
			float2(p.x, p.y),
			float2(p.x + 1.0f, p.y + 1.0f),
			float2(p.x + 1.0f, p.y)
		};
		props.extent.set(p.x, p.y, p.x + 1.0f, p.y + 1.0f);
		draw_style::draw(props, args...);
	}
};
template <class draw_style>
struct gfx2d::point_compute_line {
	template<typename... Args> static void compute_and_draw(const float2& start_pnt,
															const float2& end_pnt,
															const Args&... args) {
		compute_and_draw(start_pnt, end_pnt, 1.0f, args...);
	}
	template<typename... Args> static void compute_and_draw(const float2& start_pnt,
															const float2& end_pnt,
															const float& thickness,
															const Args&... args) {
		primitive_properties props(GL_TRIANGLE_STRIP);
		
		const float half_thickness = thickness * 0.5f;
		float x1 = start_pnt.x, x2 = end_pnt.x, y1 = start_pnt.y, y2 = end_pnt.y;
		
		// add half a pixel if this is a horizontal/vertical line - this is necessary to get
		// sharp lines (anti-aliasing would 50/50 distribute the color to two pixels otherwise)
		if(FLOAT_EQ(x1, x2)) {
			x1 += 0.5f;
			x2 += 0.5f;
		}
		else if(FLOAT_EQ(y1, y2)) {
			y1 += 0.5f;
			y2 += 0.5f;
		}
		// else: diagonal
		
		// swap points if first point is below second point
		if(y2 < y1) {
			swap(x1, x2);
			swap(y1, y2);
		}
		
		// compute line direction and rotate by 90° to the left + right and multiply
		// by the half thickness while we're at it to get the correct offset
		const float2 line_dir(float2(x2 - x1, y2 - y1).normalize() * half_thickness);
		const float2 offset_rot_left(-line_dir.y, line_dir.x);
		const float2 offset_rot_right(line_dir.y, -line_dir.x);
		
		props.points.emplace_back(x2 + offset_rot_right.x, y2 + offset_rot_right.y);
		props.points.emplace_back(x1 + offset_rot_right.x, y1 + offset_rot_right.y);
		props.points.emplace_back(x2 + offset_rot_left.x, y2 + offset_rot_left.y);
		props.points.emplace_back(x1 + offset_rot_left.x, y1 + offset_rot_left.y);
		
		//
		props.extent.set(std::min(start_pnt.x, end_pnt.x), std::min(start_pnt.y, end_pnt.y),
						 std::max(start_pnt.x, end_pnt.x), std::max(start_pnt.y, end_pnt.y));
		draw_style::draw(props, args...);
	}
};
template <class draw_style>
struct gfx2d::point_compute_triangle {
	template<typename... Args> static void compute_and_draw(const float2& p0,
															const float2& p1,
															const float2& p2,
															const Args&... args) {
		primitive_properties props(GL_TRIANGLES);
		props.points = { p0, p1, p2 };
		props.extent.set(std::min(p0.x, std::min(p1.x, p2.x)), std::min(p0.y, std::min(p1.y, p2.y)),
						 std::max(p0.x, std::min(p1.x, p2.x)), std::max(p0.y, std::min(p1.y, p2.y)));
		draw_style::draw(props, args...);
	}
};
template <class draw_style>
struct gfx2d::point_compute_rectangle {
	template<typename... Args> static void compute_and_draw(const rect& r,
															const Args&... args) {
		primitive_properties props(GL_TRIANGLE_STRIP);
		props.points = {
			float2(r.x1, r.y2),
			float2(r.x1, r.y1),
			float2(r.x2, r.y2),
			float2(r.x2, r.y1),
		};
		props.extent.set(std::min(r.x1, r.x2), std::min(r.y1, r.y2),
						 std::max(r.x1, r.x2), std::max(r.y1, r.y2));
		props.border_connect_start_end_point = 1;
		props.border_swap_strip_points = 1;
		draw_style::draw(props, args...);
	}
};
template <class draw_style>
struct gfx2d::point_compute_rounded_rectangle {
	template<typename... Args> static void compute_and_draw(const rect& r,
															const float& radius,
															const CORNER corners,
															const Args&... args) {
		primitive_properties props(GL_TRIANGLE_FAN);
		props.has_mid_point = 1;
		
		// just in case ...
		if(corners == CORNER::NONE) {
			point_compute_rectangle<draw_style>::compute_and_draw(r, args...);
			return;
		}
		
		// start off with the mid point
		const float2 mid_point((r.x1 + r.x2) / 2.0f,
							   (r.y1 + r.y2) / 2.0f);
		props.points.emplace_back(mid_point);
		
		// 0: rt, 90: rb, 180: lb, 270: lt
		for(ssize_t i = 0; i < 4; i++) {
			float2 corner_point(i < 2 ? r.x2 : r.x1, (i == 0 || i == 3) ? r.y1 : r.y2);
			if((unsigned int)corners & (1 << i)) {
				// if this is a rounded corner, add 90° circle sector for that corner
				const size_t cur_size = props.points.size();
				gfx2d::compute_ellipsoid_points(props.points, radius, radius,
												float(i) * 90.0f, float(i+1) * 90.0f);
				
				corner_point.x += (i < 2 ? -radius : radius);
				corner_point.y += (i == 0 || i == 3 ? radius : -radius);
				for(size_t j = cur_size; j < props.points.size(); j++) {
					props.points[j] += corner_point;
				}
			}
			else {
				// else: just add the corner point
				props.points.emplace_back(corner_point);
			}
		}
		
		// add first outer point so we have a complete "circle"
		props.points.emplace_back(props.points[1]);
		props.has_equal_start_end_point = 1;
		
		//
		props.extent.set(std::min(r.x1, r.x2), std::min(r.y1, r.y2),
						 std::max(r.x1, r.x2), std::max(r.y1, r.y2));
		
		draw_style::draw(props, args...);
		
	}
};
template <class draw_style>
struct gfx2d::point_compute_circle {
	template<typename... Args> static void compute_and_draw(const float2& p,
															const float radius_lr,
															const float radius_tb,
															const Args&... args) {
		primitive_properties props(GL_TRIANGLE_FAN);
		props.has_mid_point = 1;
		props.points.emplace_back(p);
		
		gfx2d::compute_ellipsoid_points(props.points, radius_lr, radius_tb, 0.0f, 360.0f);
		
		for(size_t i = 1; i < props.points.size(); i++) {
			props.points[i] += props.points[0];
		}
		
		props.extent.set(p.x - radius_lr, p.y - radius_tb,
						 p.x + radius_lr, p.y + radius_tb);
		
		draw_style::draw(props, args...);
	}
};
template <class draw_style>
struct gfx2d::point_compute_circle_sector {
	template<typename... Args> static void compute_and_draw(const float2& p,
															const float radius_lr,
															const float radius_tb,
															const float start_angle,
															const float end_angle,
															const Args&... args) {
		primitive_properties props(GL_TRIANGLE_FAN);
		props.has_mid_point = 1;
		props.points.emplace_back(p);
		
		gfx2d::compute_ellipsoid_points(props.points, radius_lr, radius_tb, start_angle, end_angle);
		
		for(size_t i = 1; i < props.points.size(); i++) {
			props.points[i] += props.points[0];
		}
		
		props.extent.set(p.x - radius_lr, p.y - radius_tb,
						 p.x + radius_lr, p.y + radius_tb);
		
		draw_style::draw(props, args...);
	}
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/*
 *	// draw style interface:
 *	struct draw_style_interface {
 *		static void draw(const primitive_properties& props, ...args...);
 *	};
 */

struct gfx2d::draw_style_fill {
	static void draw(const primitive_properties& props,
					 const float4& color) {
		//
		simple_shd->use();
		simple_shd->uniform("mvpm", *engine::get_mvp_matrix());
		simple_shd->uniform("in_color", color);
		
		// draw
		upload_points_and_draw(simple_shd, props);
	}
};
struct gfx2d::draw_style_gradient {
	static void draw(const primitive_properties& props,
					 const gfx2d::GRADIENT_TYPE type,
					 const float4& stops,
					 const vector<float4>& colors) {
		// draw
		const string option = gradient_type_to_string(type);
		
		gradient_shd->use(option);
		gradient_shd->uniform("mvpm", *engine::get_mvp_matrix());
		
		const size_t color_count = std::min(colors.size(), size_t(4));
		gradient_shd->uniform("gradients", &colors[0], color_count);
		gradient_shd->uniform("stops", stops);
		gradient_shd->uniform("extent", props.extent);
		
		// draw
		upload_points_and_draw(gradient_shd, props);
	}
};
struct gfx2d::draw_style_texture {
	// make all texture functions accept a2e_texture and forward them to opengl GLuint texture functions
	template<typename... Args> static void draw(const primitive_properties& props,
												const a2e_texture texture,
												const Args&... args) {
		draw(props, texture->tex(), args...);
	}
	
	// texture 2d
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const coord bottom_left = coord(0.0f),
					 const coord top_right = coord(1.0f),
					 const float draw_depth = 0.0f) {
		draw(props, texture, false, 0.0f, bottom_left, top_right, draw_depth, "#");
	}
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const bool passthrough,
					 const coord bottom_left = coord(0.0f),
					 const coord top_right = coord(1.0f),
					 const float draw_depth = 0.0f) {
		draw(props, texture, false, 0.0f, bottom_left, top_right, draw_depth,
			 (passthrough ? "passthrough" : "#"));
	}
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const float4 mul_color,
					 const float4 add_color,
					 const coord bottom_left = coord(0.0f),
					 const coord top_right = coord(1.0f),
					 const float draw_depth = 0.0f) {
		draw(props, texture, false, 0.0f, mul_color, add_color, bottom_left, top_right, draw_depth, "madd_color");
	}
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const float4 mul_color,
					 const float4 add_color,
					 const gfx2d::GRADIENT_TYPE type,
					 const float4& gradient_stops,
					 const vector<float4>& gradient_colors,
					 const float4 gradient_mul_interpolator = float4(0.5f),
					 const float4 gradient_add_interpolator = float4(0.0f),
					 const coord bottom_left = coord(0.0f),
					 const coord top_right = coord(1.0f),
					 const float draw_depth = 0.0f) {
		const string option = gradient_type_to_string(type);
		draw(props, texture, false, 0.0f, mul_color, add_color, gradient_stops, gradient_colors, gradient_mul_interpolator,
			 gradient_add_interpolator, bottom_left, top_right, draw_depth, option);
	}
	
	// texture 2d array
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const float layer,
					 const coord bottom_left = coord(0.0f),
					 const coord top_right = coord(1.0f),
					 const float draw_depth = 0.0f) {
		draw(props, texture, true, layer, bottom_left, top_right, draw_depth, "#");
	}
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const float layer,
					 const bool passthrough,
					 const coord bottom_left = coord(0.0f),
					 const coord top_right = coord(1.0f),
					 const float draw_depth = 0.0f) {
		draw(props, texture, true, layer, bottom_left, top_right, draw_depth,
			 (passthrough ? "passthrough" : "#"));
	}
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const float layer,
					 const float4 mul_color,
					 const float4 add_color,
					 const coord bottom_left = coord(0.0f),
					 const coord top_right = coord(1.0f),
					 const float draw_depth = 0.0f) {
		draw(props, texture, true, layer, mul_color, add_color, bottom_left, top_right, draw_depth, "madd_color");
	}
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const float layer,
					 const float4 mul_color,
					 const float4 add_color,
					 const gfx2d::GRADIENT_TYPE type,
					 const float4& gradient_stops,
					 const vector<float4>& gradient_colors,
					 const float4 gradient_mul_interpolator = float4(0.5f),
					 const float4 gradient_add_interpolator = float4(0.0f),
					 const coord bottom_left = coord(0.0f),
					 const coord top_right = coord(1.0f),
					 const float draw_depth = 0.0f) {
		const string option = gradient_type_to_string(type);
		draw(props, texture, true, layer, mul_color, add_color, gradient_stops, gradient_colors, gradient_mul_interpolator,
			 gradient_add_interpolator, bottom_left, top_right, draw_depth, option);
	}
	
protected:
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const bool is_tex_array,
					 const float layer,
					 const coord bottom_left,
					 const coord top_right,
					 const float draw_depth,
					 const string& option) {
		texture_shd->use(option, (is_tex_array ? set<string> { "*tex_array" } : set<string> {}));
		const matrix4f mvpm(matrix4f().translate(0.0f, 0.0f, draw_depth) * *engine::get_mvp_matrix());
		texture_shd->uniform("mvpm", mvpm);
		texture_shd->uniform("extent", props.extent);
		texture_shd->uniform("orientation", float4(bottom_left.x, bottom_left.y, top_right.x, top_right.y));
		texture_shd->texture("tex", texture, is_tex_array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D);
		if(is_tex_array) texture_shd->uniform("layer", layer);
		
		// draw
		upload_points_and_draw(texture_shd, props);
	}
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const bool is_tex_array,
					 const float layer,
					 const float4 mul_color,
					 const float4 add_color,
					 const coord bottom_left,
					 const coord top_right,
					 const float draw_depth,
					 const string& option) {
		texture_shd->use(option, (is_tex_array ? set<string> { "*tex_array" } : set<string> {}));
		const matrix4f mvpm(matrix4f().translate(0.0f, 0.0f, draw_depth) * *engine::get_mvp_matrix());
		texture_shd->uniform("mvpm", mvpm);
		texture_shd->uniform("extent", props.extent);
		texture_shd->uniform("orientation", float4(bottom_left.x, bottom_left.y, top_right.x, top_right.y));
		texture_shd->texture("tex", texture, is_tex_array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D);
		if(is_tex_array) texture_shd->uniform("layer", layer);
		
		texture_shd->uniform("mul_color", mul_color);
		texture_shd->uniform("add_color", add_color);
		
		// draw
		upload_points_and_draw(texture_shd, props);
	}
	static void draw(const primitive_properties& props,
					 const GLuint texture,
					 const bool is_tex_array,
					 const float layer,
					 const float4 mul_color,
					 const float4 add_color,
					 const float4& gradient_stops,
					 const vector<float4>& gradient_colors,
					 const float4 gradient_mul_interpolator,
					 const float4 gradient_add_interpolator,
					 const coord bottom_left,
					 const coord top_right,
					 const float draw_depth,
					 const string& option) {
		texture_shd->use(option, (is_tex_array ? set<string> { "*tex_array" } : set<string> {}));
		const matrix4f mvpm(matrix4f().translate(0.0f, 0.0f, draw_depth) * *engine::get_mvp_matrix());
		texture_shd->uniform("mvpm", mvpm);
		texture_shd->uniform("extent", props.extent);
		texture_shd->uniform("orientation", float4(bottom_left.x, bottom_left.y, top_right.x, top_right.y));
		texture_shd->texture("tex", texture, is_tex_array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D);
		if(is_tex_array) texture_shd->uniform("layer", layer);
		
		texture_shd->uniform("mul_color", mul_color);
		texture_shd->uniform("add_color", add_color);
		
		texture_shd->uniform("gradient_mul_interpolator", gradient_mul_interpolator);
		texture_shd->uniform("gradient_add_interpolator", gradient_add_interpolator);
		
		const size_t color_count = std::min(gradient_colors.size(), size_t(4));
		texture_shd->uniform("gradients", &gradient_colors[0], color_count);
		texture_shd->uniform("stops", gradient_stops);
		
		// draw
		upload_points_and_draw(texture_shd, props);
	}
};
template <class draw_style_next>
struct gfx2d::draw_style_border {
	//! note: this requires an additional draw_style that will do the actual drawing
	template<typename... Args> static void draw(const primitive_properties& props,
												const float thickness,
												const Args&... args) {
		// duplicate and extrude/offset
		primitive_properties border_props(GL_TRIANGLE_STRIP);
		border_props.extent = props.extent;
		border_props.flags = props.flags;
		border_props.has_mid_point = 0;
		const float2 center((props.extent.x + props.extent.z) * 0.5f,
							(props.extent.y + props.extent.w) * 0.5f);
		const vector<float2>* points = &props.points;
		const size_t orig_point_count = points->size();
		border_props.points.reserve(orig_point_count * 2 + (props.border_connect_start_end_point ? 2 : 0)); // we'll need twice as much
		
		vector<float2> swapped_points;
		if(props.border_swap_strip_points) {
			swapped_points.assign(cbegin(*points), cend(*points));
			std::swap(swapped_points[2], swapped_points[3]);
			points = &swapped_points;
		}
		
		for(size_t i = props.has_mid_point; // ignore mid point(s) if it has one
			i < orig_point_count; i++) {
			// to do this correctly, we need the previous and next point ...
			const float2& cur_point = (*points)[i];
			
			// (mp/#0) -> (sp/#1) -> #2 -> #3 -> ... -> #n-2 -> (ep/#n-1)
			const float2& prev_point = (*points)[i > props.has_mid_point ? i-1 : orig_point_count - (1 + props.has_mid_point + props.has_equal_start_end_point)];
			const float2& next_point = (*points)[i < (orig_point_count-1) ? i+1 : (props.has_mid_point * 2) + props.has_equal_start_end_point];
			
			// ... and compute the normal of those three points (from the two vectors outgoing from the current point)
			const float2 v0((prev_point - cur_point).normalize());
			const float2 v1((next_point - cur_point).normalize());
			const float2 normal((v0 + v1).normalize());
			
			//
			border_props.points.emplace_back(cur_point);
			border_props.points.emplace_back(cur_point + normal * thickness);
		}
		
		if(props.border_connect_start_end_point) {
			border_props.points.emplace_back(border_props.points[0]);
			border_props.points.emplace_back(border_props.points[1]);
		}
		
		// no point in rendering no points
		if(border_props.points.empty()) return;
		
		// draw (note: this will always render a triangle strip)
		draw_style_next::draw(border_props, args...);
	}
};

#endif
