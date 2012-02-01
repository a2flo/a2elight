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

static gl3shader simple_shd = NULL;

/*! there is no function currently
 */
gfx::gfx(engine* e_) {
	gfx::e = e_;
	gfx::exts = NULL;
	gfx::shd = NULL;
	
	vbo_fullscreen_triangle = 0;
	vbo_fullscreen_quad = 0;
	vbo_primitive = 0;
	vbo_colors = 0;
	vbo_coords = 0;
	vao_primitive = 0;
}

/*! there is no function currently
 */
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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gfx::draw_textured_fullscreen_triangle() const {
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_triangle);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_triangle);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gfx::draw_fullscreen_quad() const {
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_quad);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
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
	primitive_draw(data, size, vertex_size, vertex_type, float4(1.0f, 1.0f, 1.0f, 0.0f),
				   primitive_type, count);
}

void gfx::primitive_draw(void* data, const size_t& size,
						 const GLsizei& vertex_size, const GLenum vertex_type,
						 const float4& color,
						 const GLenum primitive_type, const GLsizei& count) {
	simple_shd->use();
	simple_shd->uniform("mvpm", *e->get_mvp_matrix());
	simple_shd->uniform("in_color", float4(color.r, color.g, color.b, 1.0f - color.a));
	glBindBuffer(GL_ARRAY_BUFFER, vbo_primitive);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STREAM_DRAW);
	glVertexAttribPointer((GLuint)simple_shd->get_attribute_position("in_vertex"),
						  vertex_size, vertex_type, GL_FALSE, 0, NULL);
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
						  vertex_size, vertex_type, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray((GLuint)simple_shd->get_attribute_position("in_vertex"));
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
	glBufferData(GL_ARRAY_BUFFER, color_count*sizeof(float4), colors, GL_STREAM_DRAW);
	glVertexAttribPointer((GLuint)simple_shd->get_attribute_position("in_color"),
						  4, GL_FLOAT, GL_FALSE, 0, NULL);
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
								   texture);
}

void gfx::draw_textured_color_rectangle(const gfx::rect& rectangle,
										const coord& bottom_left, const coord& top_right,
										const float4& color,
										GLuint texture) {
	textured_depth_color_rectangle(rectangle, bottom_left, top_right,
								   0.0f, false,
								   color, true, float4(0.0f), false,
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
								   texture);
}


void gfx::draw_textured_depth_rectangle(const gfx::rect& rectangle,
										const coord& bottom_left, const coord& top_right,
										const float& depth,
										GLuint texture) {
	textured_depth_color_rectangle(rectangle, bottom_left, top_right,
								   depth, true,
								   float4(1.0f), false, float4(0.0f), false,
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
								   texture);
}

void gfx::textured_depth_color_rectangle(const gfx::rect& rectangle,
										 const coord& bottom_left, const coord& top_right,
										 const float& depth, const bool use_depth,
										 const float4& mul_color,
										 const bool use_mul_color,
										 const float4& add_color,
										 const bool use_add_color,
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
						  3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray((GLuint)simple_shd->get_attribute_position("in_vertex"));
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_coords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(coord)*4, rect_coords, GL_STREAM_DRAW);
	glVertexAttribPointer((GLuint)simple_shd->get_attribute_position("in_tex_coord"),
						  2, GL_FLOAT, GL_FALSE, 0, NULL);
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
void gfx::draw_point(pnt* point, unsigned int color) {
	pnt p(point->x + 1, point->y); // point must be offset by 1 x
	glEnable(GL_BLEND);
	primitive_draw(&p, sizeof(pnt), 2, GL_UNSIGNED_INT,
				   float4(float((color >> 16) & 0xFF) / 255.0f,
						  float((color >> 8) & 0xFF) / 255.0f,
						  float(color & 0xFF) / 255.0f,
						  float((color >> 24) & 0xFF) / 255.0f),
				   GL_POINTS, 1);
	glDisable(GL_BLEND);
}

/*! draws a line
 *  @param point1 the position of the first point
 *  @param point2 the position of the second point
 *  @param color the color of the line
 */
void gfx::draw_line(pnt* point1, pnt* point2, unsigned int color) {
	draw_line(point1->x, point1->y, point2->x, point2->y, color);
}

void gfx::draw_line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color) {
	glEnable(GL_BLEND);
		
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
				   float4(float((color >> 16) & 0xFF) / 255.0f,
						  float((color >> 8) & 0xFF) / 255.0f,
						  float(color & 0xFF) / 255.0f,
						  float((color >> 24) & 0xFF) / 255.0f),
				   GL_TRIANGLE_STRIP, 4);
	
	glDisable(GL_BLEND);
}

/*! draws a line into a 3d space
 *  @param v1 the position of the first vertex
 *  @param v2 the position of the second vertex
 *  @param color the color of the line
 */
void gfx::draw_3d_line(const float3& v1, const float3& v2, unsigned int color) {
	float3 line3d[2];
	line3d[0] = v1;
	line3d[1] = v2;
	primitive_draw(line3d, sizeof(float3)*2, 3, GL_FLOAT,
				   float4(float((color >> 16) & 0xFF) / 255.0f,
						  float((color >> 8) & 0xFF) / 255.0f,
						  float(color & 0xFF) / 255.0f,
						  float((color >> 24) & 0xFF) / 255.0f),
				   GL_LINES, 2);
}

/*! draws a rectangle
 *  @param rectangle the rectangle itself
 *  @param color the color of the rectangle
 */
void gfx::draw_rectangle(gfx::rect* rectangle, unsigned int color) {
	draw_line(rectangle->x1, rectangle->y1, rectangle->x2, rectangle->y1, color);
	draw_line(rectangle->x1, rectangle->y2, rectangle->x2, rectangle->y2, color);
	draw_line(rectangle->x1, rectangle->y1, rectangle->x1, rectangle->y2, color);
	draw_line(rectangle->x2, rectangle->y1, rectangle->x2, rectangle->y2, color);
}

/*! draws a rectangle
 *  @param p1 left top point of the rectangle
 *  @param p2 right bottom point of the rectangle
 *  @param color the color of the rectangle
 */
void gfx::draw_rectangle(pnt* p1, pnt* p2, unsigned int color) {
	draw_line(p1->x, p1->y, p2->x, p1->y, color);
	draw_line(p1->x, p2->y, p2->x, p2->y, color);
	draw_line(p1->x, p1->y, p1->x, p2->y, color);
	draw_line(p2->x, p1->y, p2->x, p2->y, color);
}

/*! draws a two colored rectangle
 *! left + top border = first color
 *! right + bottom border = second color
 *  @param rectangle the rectangle itself
 *  @param color1 the first color of the rectangle
 *  @param color2 the second color of the rectangle
 */
void gfx::draw_2colored_rectangle(gfx::rect* rectangle,
					unsigned int color1, unsigned int color2) {
	draw_line(rectangle->x1, rectangle->y1, rectangle->x2, rectangle->y1, color1);
	draw_line(rectangle->x1, rectangle->y1, rectangle->x1, rectangle->y2, color1);
	draw_line(rectangle->x1, rectangle->y2, rectangle->x2, rectangle->y2, color2);
	draw_line(rectangle->x2, rectangle->y1, rectangle->x2, rectangle->y2, color2);
}

/*! draws a filled rectangle
 *  @param rectangle the rectangle itself
 *  @param color the color of the filled rectangle
 */
void gfx::draw_filled_rectangle(gfx::rect* rectangle, unsigned int color) {
	points[0].set(rectangle->x1, rectangle->y2 + 1);
	points[1].set(rectangle->x1, rectangle->y1);
	points[2].set(rectangle->x2 + 1, rectangle->y2 + 1);
	points[3].set(rectangle->x2 + 1, rectangle->y1);
	primitive_draw(points, sizeof(pnt)*4, 2, GL_UNSIGNED_INT,
				   float4(float((color >> 16) & 0xFF) / 255.0f,
						  float((color >> 8) & 0xFF) / 255.0f,
						  float(color & 0xFF) / 255.0f,
						  float((color >> 24) & 0xFF) / 255.0f),
				   GL_TRIANGLE_STRIP, 4);
}

void gfx::draw_fade_rectangle(gfx::rect* rectangle, unsigned int color1, unsigned int color2, FADE_TYPE ft) {
	glEnable(GL_BLEND);
	float4 colors[4];
	points[0].set(rectangle->x1, rectangle->y2 + 1);
	points[1].set(rectangle->x1, rectangle->y1);
	points[2].set(rectangle->x2 + 1, rectangle->y2 + 1);
	points[3].set(rectangle->x2 + 1, rectangle->y1);
	switch(ft) {
		case gfx::FT_HORIZONTAL:
			colors[3].set(float((color1 >> 16) & 0xFF) / 255.0f,
						  float((color1 >> 8) & 0xFF) / 255.0f,
						  float(color1 & 0xFF) / 255.0f,
						  1.0f - float((color1 >> 24) & 0xFF) / 255.0f);
			colors[1] = colors[3];
			colors[0].set(float((color2 >> 16) & 0xFF) / 255.0f,
						  float((color2 >> 8) & 0xFF) / 255.0f,
						  float(color2 & 0xFF) / 255.0f,
						  1.0f - float((color2 >> 24) & 0xFF) / 255.0f);
			colors[2] = colors[0];
			break;
		case gfx::FT_VERTICAL:
			colors[1].set(float((color1 >> 16) & 0xFF) / 255.0f,
						  float((color1 >> 8) & 0xFF) / 255.0f,
						  float(color1 & 0xFF) / 255.0f,
						  1.0f - float((color1 >> 24) & 0xFF) / 255.0f);
			colors[0] = colors[1];
			colors[2].set(float((color2 >> 16) & 0xFF) / 255.0f,
						  float((color2 >> 8) & 0xFF) / 255.0f,
						  float(color2 & 0xFF) / 255.0f,
						  1.0f - float((color2 >> 24) & 0xFF) / 255.0f);
			colors[3] = colors[2];
			break;
		case gfx::FT_DIAGONAL: {
			unsigned int avg = get_average_color(color1, color2);
			colors[3].set(float((avg >> 16) & 0xFF) / 255.0f,
						  float((avg >> 8) & 0xFF) / 255.0f,
						  float(avg & 0xFF) / 255.0f,
						  1.0f - float((avg >> 24) & 0xFF) / 255.0f);
			colors[1].set(float((color1 >> 16) & 0xFF) / 255.0f,
						  float((color1 >> 8) & 0xFF) / 255.0f,
						  float(color1 & 0xFF) / 255.0f,
						  1.0f - float((color1 >> 24) & 0xFF) / 255.0f);
			colors[0] = colors[3];
			colors[2].set(float((color2 >> 16) & 0xFF) / 255.0f,
						  float((color2 >> 8) & 0xFF) / 255.0f,
						  float(color2 & 0xFF) / 255.0f,
						  1.0f - float((color2 >> 24) & 0xFF) / 255.0f);
		}
		break;
	}
	primitive_draw_colored(points, sizeof(pnt)*4, 2, GL_UNSIGNED_INT,
						   colors, 4,
						   GL_TRIANGLE_STRIP, 4);
	glDisable(GL_BLEND);
}

unsigned int gfx::get_average_color(unsigned int color1, unsigned int color2) {
	unsigned int tmp = 0, ret = 0;
	tmp = ((color2>>24) & 0xFF) > ((color1>>24) & 0xFF)?
		((color1>>24) & 0xFF) + ((((color2>>24) & 0xFF) - ((color1>>24) & 0xFF)) / 2) :
		((color2>>24) & 0xFF) + ((((color1>>24) & 0xFF) - ((color2>>24) & 0xFF)) / 2);
	ret += tmp << 24;
	tmp = ((color2>>16) & 0xFF) > ((color1>>16) & 0xFF)?
		((color1>>16) & 0xFF) + ((((color2>>16) & 0xFF) - ((color1>>16) & 0xFF)) / 2) :
		((color2>>16) & 0xFF) + ((((color1>>16) & 0xFF) - ((color2>>16) & 0xFF)) / 2);
	ret += tmp << 16;
	tmp = ((color2>>8) & 0xFF) > ((color1>>8) & 0xFF)?
		((color1>>8) & 0xFF) + ((((color2>>8) & 0xFF) - ((color1>>8) & 0xFF)) / 2) :
		((color2>>8) & 0xFF) + ((((color1>>8) & 0xFF) - ((color2>>8) & 0xFF)) / 2);
	ret += tmp << 8;
	tmp = (color2 & 0xFF) > (color1 & 0xFF)?
		(color1 & 0xFF) + (((color2 & 0xFF) - (color1 & 0xFF)) / 2) :
		(color2 & 0xFF) + (((color1 & 0xFF) - (color2 & 0xFF)) / 2);
	ret += tmp;
	return ret;
}

/*! returns the sdl_color that we get from the function arguments and the screen surface
 *  @param red how much red (0 - 255)
 *  @param green how much green (0 - 255)
 *  @param blue how much blue (0 - 255)
 */
unsigned int gfx::get_color(unsigned int red, unsigned int green, unsigned int blue) {
	return (unsigned int)((red & 0xFF << 16) + (green & 0xFF << 8) + (blue & 0xFF));
}

/*! returns the sdl_color that we get from the function arguments and the screen surface - OBSOLETE?
 *  @param rgb red, green and blue in one value
 */
unsigned int gfx::get_color(unsigned int rgb) {
	unsigned int red = (rgb & 0xFF0000) >> 16;
	unsigned int green = (rgb & 0x00FF00) >> 8;
	unsigned int blue = rgb & 0x0000FF;
	return (unsigned int)gfx::get_color(red, green, blue);
}

/*! makes a rectangle out of 2 points
 *  @param rectangle a pointer to a rectangle object
 *  @param x1 x coord point 1
 *  @param y1 y coord point 1
 *  @param x2 x coord point 2
 *  @param y2 y coord point 2
 */
void gfx::pnt_to_rect(gfx::rect* rectangle, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
	rectangle->x1 = x1;
	rectangle->y1 = y1;
	rectangle->x2 = x2;
	rectangle->y2 = y2;
}

/*! makes a rectangle out of 2 points
 *  @param rectangle a pointer to a rectangle object
 *  @param x1 x coord point 1
 *  @param y1 y coord point 1
 *  @param x2 x coord point 2
 *  @param y2 y coord point 2
 */
void gfx::ipnt_to_rect(gfx::rect* rectangle, int x1, int y1, int x2, int y2) {
	rectangle->x1 = (unsigned int)x1;
	rectangle->y1 = (unsigned int)y1;
	rectangle->x2 = (unsigned int)x2;
	rectangle->y2 = (unsigned int)y2;
}

/*! makes a rectangle out of 2 points and returns it
 *  @param x1 x coord point 1
 *  @param y1 y coord point 1
 *  @param x2 x coord point 2
 *  @param y2 y coord point 2
 */
gfx::rect* gfx::pnt_to_rect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
	gfx::rect* r = new gfx::rect();
	r->x1 = x1;
	r->y1 = y1;
	r->x2 = x2;
	r->y2 = y2;
	return r;
}

/*! makes a point out of 2 coords
 *  @param point a pointer to a pnt object
 *  @param x x coordinate
 *  @param y y coordinate
 */
void gfx::coord_to_pnt(pnt* point, unsigned int x, unsigned int y) {
	point->x = x;
	point->y = y;
}

/*! makes a point out of 2 coords
 *  @param point a pointer to a pnt object
 *  @param x x coordinate
 *  @param y y coordinate
 */
void gfx::coord_to_ipnt(ipnt* point, int x, int y) {
	point->x = x;
	point->y = y;
}

/*! makes a point out of 2 coords and returns it
 *  @param x x cord
 *  @param y y cord
 */
pnt* gfx::coord_to_pnt(unsigned int x, unsigned int y) {
	pnt* p = new pnt();
	p->x = x;
	p->y = y;
	return p;
}

/*! makes a point out of 2 coords and returns it
 *  @param x x cord
 *  @param y y cord
 */
ipnt* gfx::coord_to_ipnt(int x, int y) {
	ipnt* p = new ipnt();
	p->x = x;
	p->y = y;
	return p;
}

/*! returns true if point is in rectangle
 *  @param rectangle the rectangle
 *  @param point the point we want to test
 */
bool gfx::is_pnt_in_rectangle(gfx::rect* rectangle, pnt* point) {
	if(point->x >= rectangle->x1 && point->x <= rectangle->x2
		&& point->y >= rectangle->y1 && point->y <= rectangle->y2) {
		return true;
	}
	return false;
}

/*! returns true if point is in rectangle
 *  @param rectangle the rectangle
 *  @param x the x coordinate we want to test
 *  @param y the y coordinate we want to test
 */
bool gfx::is_pnt_in_rectangle(gfx::rect* rectangle, unsigned int x, unsigned int y) {
	if(x >= rectangle->x1 && x <= rectangle->x2 &&
		y >= rectangle->y1 && y <= rectangle->y2) {
		return true;
	}
	return false;
}
/*! returns true if point is in rectangle
 *  @param rectangle the rectangle
 *  @param point the point we want to test
 */
bool gfx::is_pnt_in_rectangle(gfx::rect* rectangle, ipnt* point) {
	if(point->x < 0 || point->y < 0) return false;
	
	if((unsigned int)point->x >= rectangle->x1 && (unsigned int)point->x <= rectangle->x2 &&
	   (unsigned int)point->y >= rectangle->y1 && (unsigned int)point->y <= rectangle->y2) {
		return true;
	}
	
	return false;
}

/*! returns true if point is in rectangle
 *  @param rectangle the rectangle
 *  @param x the x coordinate we want to test
 *  @param y the y coordinate we want to test
 */
bool gfx::is_pnt_in_rectangle(gfx::rect* rectangle, int x, int y) {
	if(x < 0 || y < 0) return false;
	
	if((unsigned int)x >= rectangle->x1 && (unsigned int)x <= rectangle->x2 &&
	   (unsigned int)y >= rectangle->y1 && (unsigned int)y <= rectangle->y2) {
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
void gfx::set_scissor(gfx::rect* rectangle) {
	gfx::set_scissor(rectangle->x1, rectangle->y1, rectangle->x2, rectangle->y2);
}

/*! sets the scissor
 *  @param x1 x1 value of the scissor box
 *  @param y1 y1 value of the scissor box
 *  @param x2 x2 value of the scissor box
 *  @param y2 y2 value of the scissor box
 */
void gfx::set_scissor(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
	glScissor(x1, y1, x2 - x1, y2 - y1);
}

//! ends/disables scissor test
void gfx::end_scissor() {
	glDisable(GL_SCISSOR_TEST);
}

void gfx::draw_bbox(extbbox* bbox, unsigned int color) {
	// not implemented atm
}

void gfx::set_blend_mode(BLEND_MODE mode) {
	switch(mode) {
		case BM_ADD:
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			break;
		case BM_PRE_MUL:
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BM_COLOR:
			glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
			break;
		case BM_DEFAULT:
		case BM_ALPHA:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
	}
}
