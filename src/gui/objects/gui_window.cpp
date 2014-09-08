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

#include "gui_window.hpp"
#include "engine.hpp"

gui_window::gui_window(const float2& buffer_size_, const float2& position_, const SURFACE_FLAGS flags_) :
gui_object(buffer_size_, position_), gui_surface(buffer_size_, position_, flags_) {
	//
}

gui_window::~gui_window() {
	// delete all children
	clear();
}

void gui_window::clear(const bool delete_children) {
	if(delete_children) {
		// delete all children (we need to copy the container, since deleting
		// a child will modify the children container)
		const auto children_copy(children);
		for(const auto& child : children_copy) {
			delete child;
		}
	}
	else {
		// just remove all children from the window
		const auto children_copy(children);
		for(const auto& child : children_copy) {
			child->set_parent(nullptr); // this will also remove the child from this window
		}
	}
	redraw();
}

void gui_window::draw() {
	if(!state.visible) return;
	
	// check if the window redraw flag is set, in which case all children are redrawn
	const bool redraw_all = state.redraw;
	state.redraw = false;
	
	// check if any child must be redrawn
	bool redraw_any = false;
	if(!redraw_all) {
		for(const auto& child : children) {
			if(child->needs_redraw()) {
				redraw_any = true;
				break;
			}
		}
		
		// nothing to draw, return
		if(!redraw_any) return;
	}
	
	//
	vector<int4> blit_rects;
	start_draw();
	if(redraw_all) {
		r->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, background_color);
	}
	else if(shared_buffer) {
		// when using a shared buffer: create a blit rectangle list of all childs that are redrawn
		for(const auto& child : children) {
			if(child->needs_redraw()) {
				const float2 child_offset(child->get_position_abs().floored());
				const float2 child_size(child->get_size_abs().ceiled());
				blit_rects.emplace_back(child_offset.x, child_offset.y,
										child_offset.x + child_size.x, child_offset.y + child_size.y);
			}
		}
	}
	r->start_2d_draw();
	
	for(const auto& child : children) {
		if(redraw_all || child->needs_redraw()) {
			child->draw();
		}
	}
	
	r->stop_2d_draw();
	if(!redraw_all && shared_buffer) {
		// manual "stop" draw -> only blit necessary rects
		glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer->fbo_id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer->resolve_buffer[0]);
		for(const auto& blit_rect : blit_rects) {
#if !defined(FLOOR_IOS) || defined(PLATFORM_X64)
			glBlitFramebuffer(blit_rect.x, blit_rect.y, blit_rect.z, blit_rect.w,
							  blit_rect.x, blit_rect.y, blit_rect.z, blit_rect.w,
							  GL_COLOR_BUFFER_BIT, GL_NEAREST);
#else
			// TODO: gles 2.0 implementation
#endif
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else stop_draw();
}

void gui_window::redraw() {
	state.redraw = true;
}

bool gui_window::needs_redraw() const {
	return state.redraw;
}

void gui_window::resize(const float2& buffer_size_) {
	gui_surface::resize(buffer_size_);
	size = buffer_size_;
	
	// update size/position values first
	compute_abs_values();
	for(const auto& child : children) {
		child->compute_abs_values();
	}
	
	// then update all attachment values
	compute_attachment_values();
	for(const auto& child : children) {
		child->compute_attachment_values();
	}
}

void gui_window::set_size(const float2& size_) {
	resize(size_);
}

void gui_window::set_position(const float2& position_) {
	position = position_;
	set_offset(position_);
	compute_abs_values();
	compute_attachment_values();
}

bool gui_window::handle_mouse_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj, const ipnt& point) {
	// substract window position (all childs positions/sizes are relative to the window position)
	const ipnt point_in_window { point - ipnt((int)floorf(position_abs.x), (int)floorf(position_abs.y)) };
	for(const auto& child : children) {
		if(child->should_handle_mouse_event(type, point_in_window)) {
			child->lock();
			if(child->handle_mouse_event(type, obj, point_in_window)) {
				child->unlock();
				child->redraw(); // atomic
				return true;
			}
			child->unlock();
		}
	}
	return false;
}

bool gui_window::handle_key_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj) {
	for(const auto& child : children) {
		if(child->handle_key_event(type, obj)) {
			child->redraw();
			return true;
		}
	}
	return false;
}

ipnt gui_window::abs_to_rel_position(const ipnt& point) const {
	ipnt p = (parent != nullptr ? parent->abs_to_rel_position(point) : point);
	p -= position_abs;
	return p;
}

ipnt gui_window::rel_to_abs_position(const ipnt& point) const {
	ipnt p = (parent != nullptr ? parent->rel_to_abs_position(point) : point);
	p += position_abs;
	return p;
}

void gui_window::set_background_color(const float4& color) {
	background_color = color;
	redraw();
}

const float4& gui_window::get_background_color() const {
	return background_color;
}
