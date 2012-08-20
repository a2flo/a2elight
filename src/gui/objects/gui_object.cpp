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

#include "gui_object.h"
#include "engine.h"
#include "gui/gui.h"
#include "threading/task.h"

gui_object::gui_object(engine* e_, const float2& size_, const float2& position_) :
e(e_), theme(e->get_gui()->get_theme()), size(size_), position(position_) {
	compute_abs_values();
}

gui_object::~gui_object() {
}

bool gui_object::handle_draw() {
	if(!state.visible) return false;
	state.redraw = false;
	return true;
}

void gui_object::redraw() {
	state.redraw = true;
}

bool gui_object::needs_redraw() const {
	return state.redraw;
}

void gui_object::compute_abs_values() {
	const float2 parent_size(parent != nullptr ?
							 parent->get_size_abs() :
							 float2(e->get_width(), e->get_height()));
	position_abs = position * parent_size;
	size_abs = size * parent_size;
	rectangle_abs.set(position_abs.x, position_abs.y,
					  position_abs.x + size_abs.x, position_abs.y + size_abs.y);
}

const float2& gui_object::get_position() const {
	return position;
}

const float2& gui_object::get_position_abs() const {
	return position_abs;
}

const float2& gui_object::get_size() const {
	return size;
}

const float2& gui_object::get_size_abs() const {
	return size_abs;
}

const rect& gui_object::get_rectangle_abs() const {
	return rectangle_abs;
}

void gui_object::set_position(const float2& position_) {
	position = position_;
	compute_abs_values();
	redraw();
}

void gui_object::set_size(const float2& size_) {
	size = size_;
	compute_abs_values();
	redraw();
}

void gui_object::set_parent(gui_object* parent_) {
	lock();
	parent = parent_;
	unlock();
}

gui_object* gui_object::get_parent() const {
	return parent;
}

void gui_object::add_child(gui_object* child) {
	lock();
	children.insert(child);
	child->set_parent(this);
	unlock();
}

void gui_object::remove_child(gui_object* child) {
	lock();
	children.erase(child);
	unlock();
}

bool gui_object::handle_mouse_event(const EVENT_TYPE& type a2e_unused, const shared_ptr<event_object>& obj a2e_unused, const ipnt& point a2e_unused) {
	return false;
}

bool gui_object::handle_key_event(const EVENT_TYPE& type a2e_unused, const shared_ptr<event_object>& obj a2e_unused) {
	return false;
}

ipnt gui_object::abs_to_rel_position(const ipnt& point) const {
	// override this in objects that contain other objects (e.g. gui_window)
	if(parent != nullptr) return parent->abs_to_rel_position(point);
	return point;
}

void gui_object::lock() {
	mutex.lock();
}

bool gui_object::try_lock() {
	return mutex.try_lock();
}

void gui_object::unlock() {
	mutex.unlock();
}

void gui_object::add_handler(handler&& handler_, GUI_EVENT type) {
	lock();
	handlers.insert(pair<GUI_EVENT, handler>(type, handler_));
	unlock();
}

void gui_object::handle(const GUI_EVENT evt) {
	lock();
	for(const auto& hndlr : handlers) {
		task::spawn([&hndlr, &evt, this]() {
			hndlr.second(evt, *this);
		});
	}
	unlock();
}

void gui_object::remove_handlers(const GUI_EVENT& type) {
	lock();
	handlers.erase(type);
	unlock();
}

void gui_object::remove_handlers() {
	lock();
	handlers.clear();
	unlock();
}
