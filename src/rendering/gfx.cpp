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

#include "gfx.h"
#include "engine.h"
#include "shader.h"

static gl3shader simple_shd = nullptr;

gfx::gfx(engine* e_) {
	gfx::e = e_;
	gfx::exts = nullptr;
	gfx::shd = nullptr;
	
	vbo_fullscreen_triangle = 0;
	vbo_fullscreen_quad = 0;
	vbo_primitive = 0;
	vbo_colors = 0;
	vbo_coords = 0;
	vao_primitive = 0;
}

gfx::~gfx() {
	if(glIsBuffer(vbo_fullscreen_triangle)) glDeleteBuffers(1, &vbo_fullscreen_triangle);
	if(glIsBuffer(vbo_fullscreen_quad)) glDeleteBuffers(1, &vbo_fullscreen_quad);
	if(glIsBuffer(vbo_primitive)) glDeleteBuffers(1, &vbo_primitive);
	if(glIsBuffer(vbo_colors)) glDeleteBuffers(1, &vbo_colors);
	if(glIsBuffer(vbo_coords)) glDeleteBuffers(1, &vbo_coords);
	if(glIsVertexArray(vao_primitive)) glDeleteVertexArrays(1, &vao_primitive);
}

void gfx::init() {
	gfx::exts = e->get_ext();
	gfx::shd = e->get_shader();
	
	// create fullscreen triangle/quad vbo
	float _fullscreen_triangle[] = { 1.0f, 1.0f, 1.0f, -3.0f, -3.0f, 1.0f };
	memcpy(fullscreen_triangle, _fullscreen_triangle, sizeof(float)*6);
	
	glGenBuffers(1, &vbo_fullscreen_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_triangle);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float2), fullscreen_triangle, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	float _fullscreen_quad[] = { -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f };
	memcpy(fullscreen_quad, _fullscreen_quad, sizeof(float)*8);
	
	glGenBuffers(1, &vbo_fullscreen_quad);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_quad);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float2), fullscreen_quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	//
	glGenBuffers(1, &vbo_primitive);
	glGenBuffers(1, &vbo_colors);
	glGenBuffers(1, &vbo_coords);
	glGenVertexArrays(1, &vao_primitive);
	
	//
	_init_shader();
}

void gfx::_init_shader() {
	// get simple shd
	simple_shd = shd->get_gl3shader("SIMPLE");
}

void gfx::draw_fullscreen_triangle() const {
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_triangle);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gfx::draw_textured_fullscreen_triangle() const {
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_triangle);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_triangle);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gfx::draw_fullscreen_quad() const {
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_quad);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint gfx::get_fullscreen_triangle_vbo() const {
	return vbo_fullscreen_triangle;
}

GLuint gfx::get_fullscreen_quad_vbo() const {
	return vbo_fullscreen_quad;
}

void gfx::draw_primitives(void* data, const size_t& size,
						  const GLsizei& vertex_size, const GLenum vertex_type,
						  const GLenum primitive_type, const GLsizei& count) {
	primitive_draw(data, size, vertex_size, vertex_type, float4(1.0f),
				   primitive_type, count);
}

void gfx::primitive_draw(void* data, const size_t& size,
						 const GLsizei& vertex_size, const GLenum vertex_type,
						 const float4& color,
						 const GLenum primitive_type, const GLsizei& count) {
	simple_shd->use();
	simple_shd->uniform("mvpm", *e->get_mvp_matrix());
	simple_shd->uniform("in_color", float4(color.r, color.g, color.b, color.a));
	glBindBuffer(GL_ARRAY_BUFFER, vbo_primitive);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STREAM_DRAW);
	glVertexAttribPointer((GLuint)simple_shd->get_attribute_position("in_vertex"),
						  vertex_size, vertex_type, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray((GLuint)simple_shd->get_attribute_position("in_vertex"));
	
	glDrawArrays(primitive_type, 0, count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gfx::primitive_draw_colored(void* data, const size_t& size,
								 const GLsizei& vertex_size, const GLenum vertex_type,
								 const float4* colors, const size_t& color_count,
								 const GLenum primitive_type, const GLsizei& count) {
	simple_shd->use("colored");
	simple_shd->uniform("mvpm", *e->get_mvp_matrix());
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_primitive);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STREAM_DRAW);
	glVertexAttribPointer((GLuint)simple_shd->get_attribute_position("in_vertex"),
						  vertex_size, vertex_type, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray((GLuint)simple_shd->get_attribute_position("in_vertex"));
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
	glBufferData(GL_ARRAY_BUFFER, color_count*sizeof(float4), colors, GL_STREAM_DRAW);
	glVertexAttribPointer((GLuint)simple_shd->get_attribute_position("in_color"),
						  4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray((GLuint)simple_shd->get_attribute_position("in_color"));
	
	glDrawArrays(primitive_type, 0, count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gfx::draw_textured_rectangle(const gfx::rect& rectangle,
								  const coord& bottom_left, const coord& top_right,
								  GLuint texture) {
	textured_depth_color_rectangle(rectangle, bottom_left, top_right,
								   0.0f, false,
								   float4(1.0f), false, float4(0.0f), false,
								   false,
								   texture);
}

void gfx::draw_textured_passthrough_rectangle(const gfx::rect& rectangle,
											  const coord& bottom_left, const coord& top_right,
											  GLuint texture) {
	textured_depth_color_rectangle(rectangle, bottom_left, top_right,
								   0.0f, false,
								   float4(1.0f), false, float4(0.0f), false,
								   true,
								   texture);
}

void gfx::draw_textured_color_rectangle(const gfx::rect& rectangle,
										const coord& bottom_left, const coord& top_right,
										const float4& color,
										GLuint texture) {
	textured_depth_color_rectangle(rectangle, bottom_left, top_right,
								   0.0f, false,
								   color, true, float4(0.0f), false,
								   false,
								   texture);
}

void gfx::draw_textured_color_rectangle(const gfx::rect& rectangle,
										const coord& bottom_left, const coord& top_right,
										const float4& mul_color,
										const float4& add_color,
										GLuint texture) {
	textured_depth_color_rectangle(rectangle, bottom_left, top_right,
								   0.0f, false,
								   mul_color, true, add_color, true,
								   false,
								   texture);
}


void gfx::draw_textured_depth_rectangle(const gfx::rect& rectangle,
										const coord& bottom_left, const coord& top_right,
										const float& depth,
										GLuint texture) {
	textured_depth_color_rectangle(rectangle, bottom_left, top_right,
								   depth, true,
								   float4(1.0f), false, float4(0.0f), false,
								   false,
								   texture);
}

void gfx::draw_textured_depth_color_rectangle(const gfx::rect& rectangle,
											  const coord& bottom_left, const coord& top_right,
											  const float& depth,
											  const float4& color,
											  GLuint texture) {
	textured_depth_color_rectangle(rectangle, bottom_left, top_right,
								   depth, true,
								   color, true, float4(0.0f), false,
								   false,
								   texture);
}

void gfx::draw_textured_depth_color_rectangle(const gfx::rect& rectangle,
											  const coord& bottom_left, const coord& top_right,
											  const float& depth,
											  const float4& mul_color,
											  const float4& add_color,
											  GLuint texture) {
	textured_depth_color_rectangle(rectangle, bottom_left, top_right,
								   depth, true,
								   mul_color, true, add_color, true,
								   false,
								   texture);
}

void gfx::textured_depth_color_rectangle(const gfx::rect& rectangle,
										 const coord& bottom_left, const coord& top_right,
										 const float& depth, const bool use_depth,
										 const float4& mul_color,
										 const bool use_mul_color,
										 const float4& add_color,
										 const bool use_add_color,
										 const bool passthrough,
										 GLuint texture) {
	if(use_mul_color) {
		if(use_add_color) {
			simple_shd->use("texture_madd_color");
			simple_shd->uniform("in_color", mul_color);
			simple_shd->uniform("add_color", add_color);
		}
		else {
			simple_shd->use("texture_mul_color");
			simple_shd->uniform("in_color", mul_color);
		}
	}
	else if(passthrough) {
		simple_shd->use("textured_passthrough");
	}
	else {
		simple_shd->use("textured");
	}
	simple_shd->uniform("mvpm", *e->get_mvp_matrix());
	simple_shd->texture("tex", texture);
	
	const float vdepth = (use_depth ? depth : 0.0f);
	verts[0].set(rectangle.x1, rectangle.y2, vdepth);
	verts[1].set(rectangle.x1, rectangle.y1, vdepth);
	verts[2].set(rectangle.x2, rectangle.y2, vdepth);
	verts[3].set(rectangle.x2, rectangle.y1, vdepth);
	
	rect_coords[0].set(bottom_left.x, top_right.y);
	rect_coords[1].set(bottom_left.x, bottom_left.y);
	rect_coords[2].set(top_right.x, top_right.y);
	rect_coords[3].set(top_right.x, bottom_left.y);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_primitive);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float3)*4, verts, GL_STREAM_DRAW);
	glVertexAttribPointer((GLuint)simple_shd->get_attribute_position("in_vertex"),
						  3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray((GLuint)simple_shd->get_attribute_position("in_vertex"));
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_coords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(coord)*4, rect_coords, GL_STREAM_DRAW);
	glVertexAttribPointer((GLuint)simple_shd->get_attribute_position("in_tex_coord"),
						  2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray((GLuint)simple_shd->get_attribute_position("in_tex_coord"));
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*! draws a point
 *  @param point the position of the point
 *  @param color the color of the point
 */
void gfx::draw_point(const pnt& point, const float4& color) {
	draw_filled_rectangle(gfx::rect(point.x, point.y, point.x, point.y), color);
}

/*! draws a line
 *  @param point1 the position of the first point
 *  @param point2 the position of the second point
 *  @param color the color of the line
 */
void gfx::draw_line(const pnt& point1, const pnt& point2, const float4& color) {
	draw_line(point1.x, point1.y, point2.x, point2.y, color);
}

void gfx::draw_line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, const float4& color) {
	// diagonal line
	if(y1 != y2 && x1 != x2) {
		// swap points if first point is below second point
		if(y2 < y1) {
			swap(x1, x2);
			swap(y1, y2);
		}
		
		// append line by one pixel ("the last pixel")
		x2 > x1 ? x2++ : x1++;
		y2 > y1 ? y2++ : y1++;
		
		points[0].x = x2;
		points[0].y = y2;
		points[1].x = x1;
		points[1].y = y1;
		points[2].x = x2+1;
		points[2].y = y2;
		points[3].x = x1+1;
		points[3].y = y1;
	}
	// horizontal/vertical line or point
	else {
		// offset/append by one pixel (to fill a whole pixel)
		x2++;
		y2++;
		
		points[0].x = x1;
		points[0].y = y2;
		points[1].x = x1;
		points[1].y = y1;
		points[2].x = x2;
		points[2].y = y2;
		points[3].x = x2;
		points[3].y = y1;
	}
	
	primitive_draw(points, sizeof(pnt)*4, 2, GL_UNSIGNED_INT,
				  color, GL_TRIANGLE_STRIP, 4);
}

/*! draws a line into a 3d space
 *  @param v1 the position of the first vertex
 *  @param v2 the position of the second vertex
 *  @param color the color of the line
 */
void gfx::draw_3d_line(const float3& v1, const float3& v2, const float4& color) {
	float3 line3d[2];
	line3d[0] = v1;
	line3d[1] = v2;
	primitive_draw(line3d, sizeof(float3)*2, 3, GL_FLOAT,
				   color, GL_LINES, 2);
}

/*! draws a rectangle
 *  @param rectangle the rectangle itself
 *  @param color the color of the rectangle
 */
void gfx::draw_rectangle(const gfx::rect& rectangle, const float4& color) {
	draw_line(rectangle.x1, rectangle.y1, rectangle.x2, rectangle.y1, color);
	draw_line(rectangle.x1, rectangle.y2, rectangle.x2, rectangle.y2, color);
	draw_line(rectangle.x1, rectangle.y1, rectangle.x1, rectangle.y2, color);
	draw_line(rectangle.x2, rectangle.y1, rectangle.x2, rectangle.y2, color);
}

/*! draws a rectangle
 *  @param p1 left top point of the rectangle
 *  @param p2 right bottom point of the rectangle
 *  @param color the color of the rectangle
 */
void gfx::draw_rectangle(const pnt& p1, const pnt& p2, const float4& color) {
	draw_line(p1.x, p1.y, p2.x, p1.y, color);
	draw_line(p1.x, p2.y, p2.x, p2.y, color);
	draw_line(p1.x, p1.y, p1.x, p2.y, color);
	draw_line(p2.x, p1.y, p2.x, p2.y, color);
}

/*! draws a two colored rectangle
 *! left + top border = first color
 *! right + bottom border = second color
 *  @param rectangle the rectangle itself
 *  @param color1 the first color of the rectangle
 *  @param color2 the second color of the rectangle
 */
void gfx::draw_2colored_rectangle(const gfx::rect& rectangle,
								  const float4& color1, const float4& color2) {
	draw_line(rectangle.x1, rectangle.y1, rectangle.x2, rectangle.y1, color1);
	draw_line(rectangle.x1, rectangle.y1, rectangle.x1, rectangle.y2, color1);
	draw_line(rectangle.x1, rectangle.y2, rectangle.x2, rectangle.y2, color2);
	draw_line(rectangle.x2, rectangle.y1, rectangle.x2, rectangle.y2, color2);
}

/*! draws a filled rectangle
 *  @param rectangle the rectangle itself
 *  @param color the color of the filled rectangle
 */
void gfx::draw_filled_rectangle(const gfx::rect& rectangle, const float4& color) {
	points[0].set(rectangle.x1, rectangle.y2 + 1);
	points[1].set(rectangle.x1, rectangle.y1);
	points[2].set(rectangle.x2 + 1, rectangle.y2 + 1);
	points[3].set(rectangle.x2 + 1, rectangle.y1);
	primitive_draw(points, sizeof(pnt)*4, 2, GL_UNSIGNED_INT,
				   color, GL_TRIANGLE_STRIP, 4);
}

void gfx::draw_gradient_rectangle(const gfx::rect& rectangle, const float4& color1, const float4& color2, const GRADIENT_TYPE gt) {
	float4 colors[4];
	points[0].set(rectangle.x1, rectangle.y2 + 1);
	points[1].set(rectangle.x1, rectangle.y1);
	points[2].set(rectangle.x2 + 1, rectangle.y2 + 1);
	points[3].set(rectangle.x2 + 1, rectangle.y1);
	switch(gt) {
		case GRADIENT_TYPE::HORIZONTAL:
			colors[3] = color1;
			colors[1] = colors[3];
			colors[0] = color2;
			colors[2] = colors[0];
			break;
		case GRADIENT_TYPE::VERTICAL:
			colors[1] = color1;
			colors[0] = colors[1];
			colors[2] = color2;
			colors[3] = colors[2];
			break;
		case GRADIENT_TYPE::DIAGONAL: {
			colors[3] = float4((color1.x + color2.x) * 0.5f,
							   (color1.y + color2.y) * 0.5f,
							   (color1.z + color2.z) * 0.5f,
							   (color1.w + color2.w) * 0.5f);
			colors[1] = color1;
			colors[0] = colors[3];
			colors[2] = color2;
		}
		break;
	}
	primitive_draw_colored(points, sizeof(pnt)*4, 2, GL_UNSIGNED_INT,
						   colors, 4, GL_TRIANGLE_STRIP, 4);
}

/*! returns true if point is in rectangle
 *  @param rectangle the rectangle
 *  @param point the point we want to test
 */
bool gfx::is_pnt_in_rectangle(const gfx::rect& rectangle, const pnt& point) {
	if(point.x >= rectangle.x1 && point.x <= rectangle.x2 &&
	   point.y >= rectangle.y1 && point.y <= rectangle.y2) {
		return true;
	}
	return false;
}

/*! returns true if point is in rectangle
 *  @param rectangle the rectangle
 *  @param x the x coordinate we want to test
 *  @param y the y coordinate we want to test
 */
bool gfx::is_pnt_in_rectangle(const gfx::rect& rectangle, const unsigned int& x, const unsigned int& y) {
	if(x >= rectangle.x1 && x <= rectangle.x2 &&
	   y >= rectangle.y1 && y <= rectangle.y2) {
		return true;
	}
	return false;
}
/*! returns true if point is in rectangle
 *  @param rectangle the rectangle
 *  @param point the point we want to test
 */
bool gfx::is_pnt_in_rectangle(const gfx::rect& rectangle, const ipnt& point) {
	if(point.x < 0 || point.y < 0) return false;
	
	if((unsigned int)point.x >= rectangle.x1 && (unsigned int)point.x <= rectangle.x2 &&
	   (unsigned int)point.y >= rectangle.y1 && (unsigned int)point.y <= rectangle.y2) {
		return true;
	}
	
	return false;
}

/*! returns true if point is in rectangle
 *  @param rectangle the rectangle
 *  @param x the x coordinate we want to test
 *  @param y the y coordinate we want to test
 */
bool gfx::is_pnt_in_rectangle(const gfx::rect& rectangle, const int& x, const int& y) {
	if(x < 0 || y < 0) return false;
	
	if((unsigned int)x >= rectangle.x1 && (unsigned int)x <= rectangle.x2 &&
	   (unsigned int)y >= rectangle.y1 && (unsigned int)y <= rectangle.y2) {
		return true;
	}
	
	return false;
}

//! begins/enables the scissor
void gfx::begin_scissor() {
	glEnable(GL_SCISSOR_TEST);
}

/*! sets the scissor
 *  @param rectangle the scissor box
 */
void gfx::set_scissor(const gfx::rect& rectangle) {
	gfx::set_scissor(rectangle.x1, rectangle.y1, rectangle.x2, rectangle.y2);
}

/*! sets the scissor
 *  @param x1 x1 value of the scissor box
 *  @param y1 y1 value of the scissor box
 *  @param x2 x2 value of the scissor box
 *  @param y2 y2 value of the scissor box
 */
void gfx::set_scissor(const unsigned int& x1, const unsigned int& y1, const unsigned int& x2, const unsigned int& y2) {
	glScissor(x1, y1, x2 - x1, y2 - y1);
}

//! ends/disables scissor test
void gfx::end_scissor() {
	glDisable(GL_SCISSOR_TEST);
}

void gfx::draw_bbox(const extbbox& bbox, const float4& color) {
	// not implemented atm (TODO)
}

void gfx::set_blend_mode(const BLEND_MODE mode) {
	switch(mode) {
		case BLEND_MODE::ADD:
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			break;
		case BLEND_MODE::PRE_MUL:
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BLEND_MODE::COLOR:
			glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
			break;
		case BLEND_MODE::DEFAULT:
		case BLEND_MODE::ALPHA:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
	}
}

void gfx::draw_circle(const pnt& p, const float& radius, const float4& color) {
	draw_ellipsoid(p, radius, radius, color);
}

void gfx::draw_ellipsoid(const pnt& p, const float& radius_lr, const float& radius_tb, const float4& color) {
	vector<float2> ell_points;
	ell_points.push_back(p);
	
	compute_ellipsoid_points(ell_points, radius_lr, radius_tb, 0.0f, 360.0f);
	for(size_t i = 1; i < ell_points.size(); i++) {
		ell_points[i] += p;
	}
	
	primitive_draw(&ell_points[0], sizeof(float2) * ell_points.size(),
				   2, GL_FLOAT,
				   color,
				   GL_TRIANGLE_FAN, ell_points.size());
}

void gfx::draw_circle_sector(const pnt& p, const float& radius, const float& start_angle, const float& end_angle, const float4& color) {
	vector<float2> sec_points;
	sec_points.push_back(p);
	
	compute_ellipsoid_points(sec_points, radius, radius, start_angle, end_angle);
	for(size_t i = 1; i < sec_points.size(); i++) {
		sec_points[i] += p;
	}
	
	primitive_draw(&sec_points[0], sizeof(float2) * sec_points.size(),
				   2, GL_FLOAT,
				   color,
				   GL_TRIANGLE_FAN, sec_points.size());
}

void gfx::draw_filled_rounded_rectangle(const gfx::rect& rectangle, const float& radius, const float4& color) {
	draw_filled_rounded_rectangle(rectangle, radius, bool4(true), color);
}

void gfx::draw_filled_rounded_rectangle(const gfx::rect& rectangle, const float& radius, const bool4& corners, const float4& color) {
	// just in case ...
	if(corners.x == false &&
	   corners.y == false &&
	   corners.z == false &&
	   corners.w == false) {
		draw_filled_rectangle(rectangle, color);
		return;
	}
	
	// start off with the mid point
	const float2 mid_point((rectangle.x1 + rectangle.x2) / 2.0f,
						   (rectangle.y1 + rectangle.y2) / 2.0f);
	vector<float2> rr_points;
	rr_points.push_back(mid_point);
	
	// 0: rt, 90: rb, 180: lb, 270: lt
	for(ssize_t i = 0; i < 4; i++) {
		float2 corner_point(i < 2 ? rectangle.x2 : rectangle.x1,
							i == 0 || i == 3 ? rectangle.y1 : rectangle.y2);
		if(corners[i]) {
			// if this is a rounded corner, add 90° circle sector for that corner
			const size_t cur_size = rr_points.size();
			compute_ellipsoid_points(rr_points, radius, radius,
									 float(i) * 90.0f, float(i+1) * 90.0f);
			
			corner_point.x += (i < 2 ? -radius : radius);
			corner_point.y += (i == 0 || i == 3 ? radius : -radius);
			for(size_t j = cur_size; j < rr_points.size(); j++) {
				rr_points[j] += corner_point;
			}
		}
		else {
			// else: just add the corner point
			rr_points.push_back(corner_point);
		}
	}
	
	// add first outer point so we have a complete "circle"
	rr_points.push_back(rr_points[1]);
	
	primitive_draw(&rr_points[0], sizeof(float2) * rr_points.size(),
				   2, GL_FLOAT,
				   color,
				   GL_TRIANGLE_FAN, rr_points.size());
}

void gfx::compute_ellipsoid_points(vector<float2>& dst_points, const float& radius_lr, const float& radius_tb, const float& start_angle, const float& end_angle) {
	//
	const float angle_size = ((end_angle - start_angle) / 360.0f);
	const float steps_per_quadrant_lr = ceilf(radius_lr); // "per 90° or 0.25 angle size"
	const float steps_per_quadrant_tb = ceilf(radius_tb);
	const float angle_offset = (start_angle / 360.0f) * (2.0f * M_PI);
	const size_t steps = (size_t)(std::max(steps_per_quadrant_lr, steps_per_quadrant_tb) * (angle_size * 4.0f));
	const float fsteps = steps-1;
	
	dst_points.reserve(dst_points.size() + steps);
	for(size_t i = 0; i < steps; i++) {
		const float fstep = ((float(i) / fsteps) * angle_size) * (2.0f * M_PI);
		const float sin_step = sinf(angle_offset + fstep);
		const float cos_step = -cosf(angle_offset + fstep);
		dst_points.push_back(float2(sin_step * radius_lr, cos_step * radius_tb));
	}
}
