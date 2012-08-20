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

#include "gui_window.h"
#include "engine.h"

gui_window::gui_window(engine* e_, const float2& buffer_size_, const float2& position_) :
gui_object(e_, buffer_size_, position_), gui_surface(e_, buffer_size_, position_) {
	//
}

gui_window::~gui_window() {
}

void gui_window::draw() {
	if(!state.visible) return;
	
	// check if the window redraw flag is set, in which case all children are redrawn
	const bool redraw_all = state.redraw;
	state.redraw = false;
	
	//
	r->start_draw(buffer);
	if(redraw_all) r->clear();
	r->start_2d_draw();
	
	for(const auto& child : children) {
		if(redraw_all || child->needs_redraw()) {
			child->draw();
		}
	}
	
	r->stop_2d_draw();
	r->stop_draw();
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
	compute_abs_values();
	for(const auto& child : children) {
		child->compute_abs_values();
	}
}

void gui_window::set_size(const float2& size_) {
	resize(size_);
}

void gui_window::set_position(const float2& position_) {
	position = position_;
	set_offset(position_);
	compute_abs_values();
}

bool gui_window::handle_mouse_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj, const ipnt& point) {
	// substract window position (all childs positions/sizes are relative to the window position)
	const ipnt point_in_window { point - ipnt(floorf(position_abs.x), floorf(position_abs.y)) };
	for(const auto& child : children) {
		if(gfx2d::is_pnt_in_rectangle(child->get_rectangle_abs(), point_in_window)) {
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
