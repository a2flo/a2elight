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

#include "gfx2d.h"
#include "engine.h"
#include "rendering/shader.h"
#include "gui/event.h"
#include "gui/event_objects.h"
#include "rendering/extensions.h"

gl3shader gfx2d::simple_shd = nullptr;
gl3shader gfx2d::gradient_shd = nullptr;
gl3shader gfx2d::texture_shd = nullptr;
engine* gfx2d::e = nullptr;
shader* gfx2d::eshd = nullptr;
ext* gfx2d::exts = nullptr;

GLuint gfx2d::vbo_primitive = 0;
static GLuint vbo_fullscreen_triangle = 0;
static GLuint vbo_fullscreen_quad = 0;
static const float fullscreen_triangle[6] = { 1.0f, 1.0f, 1.0f, -3.0f, -3.0f, 1.0f };
static const float fullscreen_quad[8] = { -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f };

event::handler gfx2d::evt_handler(&gfx2d::event_handler);

void gfx2d::init(engine* e_) {
	e = e_;
	exts = e->get_ext();
	eshd = e->get_shader();
	
	//
	e->get_event()->add_internal_event_handler(evt_handler, EVENT_TYPE::SHADER_RELOAD);
	
	// create fullscreen triangle/quad vbo
	glGenBuffers(1, &vbo_fullscreen_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_triangle);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float2), fullscreen_triangle, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &vbo_fullscreen_quad);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_quad);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float2), fullscreen_quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	//
	glGenBuffers(1, &vbo_primitive);
	
	//
	reload_shaders();
}

void gfx2d::destroy() {
	if(glIsBuffer(vbo_fullscreen_triangle)) glDeleteBuffers(1, &vbo_fullscreen_triangle);
	if(glIsBuffer(vbo_fullscreen_quad)) glDeleteBuffers(1, &vbo_fullscreen_quad);
	if(glIsBuffer(vbo_primitive)) glDeleteBuffers(1, &vbo_primitive);
	
	e->get_event()->remove_event_handler(evt_handler);
}

void gfx2d::reload_shaders() {
	// get simple shd
	gradient_shd = eshd->get_gl3shader("GFX2D_GRADIENT");
	texture_shd = eshd->get_gl3shader("GFX2D_TEXTURE");
	simple_shd = eshd->get_gl3shader("SIMPLE");
}

bool gfx2d::event_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(type == EVENT_TYPE::SHADER_RELOAD) {
		gfx2d::reload_shaders();
	}
	return true;
}

void gfx2d::draw_fullscreen_triangle() {
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_triangle);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gfx2d::draw_textured_fullscreen_triangle() {
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

void gfx2d::draw_fullscreen_quad() {
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_quad);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint gfx2d::get_fullscreen_triangle_vbo() {
	return vbo_fullscreen_triangle;
}

GLuint gfx2d::get_fullscreen_quad_vbo() {
	return vbo_fullscreen_quad;
}

void gfx2d::compute_ellipsoid_points(vector<float2>& dst_points, const float& radius_lr, const float& radius_tb, const float& start_angle, const float& end_angle) {
	//
	const float angle_size = (end_angle >= start_angle ? (end_angle - start_angle) : (360.0f + end_angle - start_angle)) / 360.0f;
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
		dst_points.emplace_back(float2(sin_step * radius_lr, cos_step * radius_tb));
	}
}

void gfx2d::upload_points_and_draw(const gl3shader& shd, const primitive_properties& props) {
	// points
	glBindBuffer(GL_ARRAY_BUFFER, vbo_primitive);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float2)*props.points.size(), &props.points[0], GL_STREAM_DRAW);
	glVertexAttribPointer((GLuint)shd->get_attribute_position("in_vertex"),
						  2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray((GLuint)shd->get_attribute_position("in_vertex"));
	
	// draw
	glDrawArrays(props.primitive_type, 0, props.points.size());
	
	// disable everything
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	shd->disable();
}

//! begins/enables the scissor
void gfx2d::begin_scissor() {
	glEnable(GL_SCISSOR_TEST);
}

/*! sets the scissor
 *  @param rectangle the scissor box
 */
void gfx2d::set_scissor(const rect& rectangle) {
	set_scissor(rectangle.x1, rectangle.y1, rectangle.x2, rectangle.y2);
}

/*! sets the scissor
 */
void gfx2d::set_scissor(const unsigned int& x1, const unsigned int& y1, const unsigned int& x2, const unsigned int& y2) {
	glScissor(x1, y1, x2 - x1, y2 - y1);
}

//! ends/disables scissor test
void gfx2d::end_scissor() {
	glDisable(GL_SCISSOR_TEST);
}

void gfx2d::set_blend_mode(const BLEND_MODE mode) {
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

/*! returns true if point is in rectangle
 *  @param rectangle the rectangle
 *  @param point the point we want to test
 */
bool gfx2d::is_pnt_in_rectangle(const rect& rectangle, const pnt& point) {
	if(point.x >= rectangle.x1 && point.x <= rectangle.x2 &&
	   point.y >= rectangle.y1 && point.y <= rectangle.y2) {
		return true;
	}
	return false;
}

/*! returns true if point is in rectangle
 *  @param rectangle the rectangle
 *  @param point the point we want to test
 */
bool gfx2d::is_pnt_in_rectangle(const rect& rectangle, const ipnt& point) {
	if(point.x < 0 || point.y < 0) return false;
	
	if(point.x >= (int)rectangle.x1 && point.x <= (int)rectangle.x2 &&
	   point.y >= (int)rectangle.y1 && point.y <= (int)rectangle.y2) {
		return true;
	}
	return false;
}
