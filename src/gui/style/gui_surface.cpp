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

#include "gui_surface.h"
#include "gui.h"
#include "engine.h"
#include "rendering/gfx2d.h"
#include "rendering/shader.h"

gui_surface::gui_surface(engine* e_, const float2& buffer_size_, const float2& offset_, const SURFACE_FLAGS flags_) :
e(e_), r(e->get_rtt()), flags(flags_), buffer_size(buffer_size_), offset(offset_) {
	glGenBuffers(1, &vbo_rectangle);
	resize(buffer_size);
}

gui_surface::~gui_surface() {
	if(buffer != nullptr) {
		r->delete_buffer(buffer);
		buffer = nullptr;
	}
	
	if(glIsBuffer(vbo_rectangle)) glDeleteBuffers(1, &vbo_rectangle);
}

void gui_surface::resize(const float2& buffer_size_) {
	uint2 buffer_size_abs_ = buffer_size * float2(e->get_width(), e->get_height());
	if(buffer != nullptr &&
	   buffer_size_abs.x == buffer_size_abs_.x && buffer_size_abs.y == buffer_size_abs_.y) {
		// same size, nothing to do here
		return;
	}
	buffer_size = buffer_size_;
	buffer_size_abs = buffer_size_abs_;
	
	if(buffer != nullptr) {
		if(shared_buffer) {
			// don't delete gui fs fbo buffers
			buffer->depth_buffer = 0;
			buffer->color_buffer = 0;
		}
		r->delete_buffer(buffer);
	}
	
	// share a "global" msaa fullscreen buffer among all surfaces and only add an additional
	// resolve/blit buffer for each surface
	if(flags == SURFACE_FLAGS::NONE &&
	   buffer_size.x == 1.0f && buffer_size.y == 1.0f) {
		const auto* fs_fbo = e->get_gui()->get_fullscreen_fbo();
		buffer = r->add_buffer(buffer_size_abs.x, buffer_size_abs.y, GL_TEXTURE_2D, TEXTURE_FILTERING::POINT,
							   rtt::TEXTURE_ANTI_ALIASING::NONE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
							   GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
		buffer->depth_buffer = fs_fbo->depth_buffer;
		buffer->color_buffer = fs_fbo->color_buffer;
		buffer->samples = fs_fbo->samples;
		buffer->anti_aliasing[0] = fs_fbo->anti_aliasing[0];
		
		glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo_id);
		glGenFramebuffers(1, &buffer->resolve_buffer[0]);
		glBindFramebuffer(GL_FRAMEBUFFER, buffer->resolve_buffer[0]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->tex[0], 0);
		glBindFramebuffer(GL_FRAMEBUFFER, A2E_DEFAULT_FRAMEBUFFER);
		
		shared_buffer = true;
	}
	else {
		buffer = r->add_buffer(buffer_size_abs.x, buffer_size_abs.y, GL_TEXTURE_2D, TEXTURE_FILTERING::POINT,
							   (flags & SURFACE_FLAGS::NO_ANTI_ALIASING) == SURFACE_FLAGS::NO_ANTI_ALIASING ?
							   rtt::TEXTURE_ANTI_ALIASING::NONE : e->get_ui_anti_aliasing(),
							   GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 1,
							   (flags & SURFACE_FLAGS::NO_DEPTH) == SURFACE_FLAGS::NO_DEPTH ?
							   rtt::DEPTH_TYPE::NONE : rtt::DEPTH_TYPE::RENDERBUFFER);
		shared_buffer = false;
	}
	
	// set blit vbo rectangle data
	set_offset(offset);
	
	//
	redraw();
}

void gui_surface::redraw() {
	do_redraw = true;
}

bool gui_surface::needs_redraw() const {
	return do_redraw;
}

rtt::fbo* gui_surface::get_buffer() const {
	return buffer;
}

const float2& gui_surface::get_buffer_size() const {
	return buffer_size;
}

void gui_surface::blit(gl3shader& shd) {
	shd->uniform("extent", extent);
	shd->texture("tex", buffer->tex[0]);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_rectangle);
	glVertexAttribPointer((GLuint)shd->get_attribute_position("in_vertex"),
						  2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray((GLuint)shd->get_attribute_position("in_vertex"));
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void gui_surface::set_offset(const float2& offset_) {
	// TODO: make this context-less (rather set the offset as a shader uniform)
	
	// set blit vbo rectangle data
	offset = offset_;
	uint2 offset_abs = offset * float2(e->get_width(), e->get_height());
	extent.set(offset_abs.x, offset_abs.y, offset_abs.x + buffer->width, offset_abs.y + buffer->height);
	const array<float2, 4> points {
		{
			float2(extent.x, extent.w),
			float2(extent.x, extent.y),
			float2(extent.z, extent.w),
			float2(extent.z, extent.y)
		}
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_rectangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * 4, &points[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

const float2& gui_surface::get_offset() const {
	return offset;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
gui_simple_callback::gui_simple_callback(ui_draw_callback& callback_, const DRAW_MODE_UI& mode_,
										 engine* e, const float2& buffer_size_, const float2& offset_,
										 const SURFACE_FLAGS flags_) :
gui_surface(e, buffer_size_, offset_, flags_), mode(mode_), callback(&callback_) {
}

void gui_simple_callback::draw() {
	if(!do_redraw) return;
	
	r->start_draw(buffer);
	r->clear();
	r->start_2d_draw();
	
	(*callback)(mode, buffer);
	
	r->stop_2d_draw();
	r->stop_draw();
	
	do_redraw = false;
}
