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

#include "gui/objects/gui_object_event.hpp"
#include "engine.hpp"
#include "gui/gui.hpp"
#include <floor/threading/task.hpp>

gui_object_event::gui_object_event() noexcept : evt(floor::get_event()) {
}

void gui_object_event::lock() {
	mutex.lock();
}

bool gui_object_event::try_lock() {
	return mutex.try_lock();
}

void gui_object_event::unlock() {
	mutex.unlock();
}

void gui_object_event::add_handler(handler&& handler_, GUI_EVENT type) {
	lock();
	handlers.emplace(type, handler_);
	unlock();
}

void gui_object_event::handle(const GUI_EVENT gui_evt) {
	lock();
	const auto range = handlers.equal_range(gui_evt);
	for(auto iter = range.first; iter != range.second; iter++) {
		task::spawn([&hndlr = *iter, gui_evt, this]() {
			hndlr.second(gui_evt, (gui_object&)*this);
		}, "evt handler");
	}
	unlock();
	
	// also add this event to the global event handler
	evt->add_event((EVENT_TYPE)gui_evt, make_shared<gui_event>(SDL_GetTicks(), (gui_object&)*this));
}

void gui_object_event::remove_handlers(const GUI_EVENT& type) {
	lock();
	handlers.erase(type);
	unlock();
}

void gui_object_event::remove_handlers() {
	lock();
	handlers.clear();
	unlock();
}

bool gui_object_event::handle_mouse_event(const EVENT_TYPE& type floor_unused, const shared_ptr<event_object>& obj floor_unused, const int2& point floor_unused) {
	return false;
}

bool gui_object_event::handle_key_event(const EVENT_TYPE& type floor_unused, const shared_ptr<event_object>& obj floor_unused) {
	return false;
}

bool gui_object_event::has_handler(const GUI_EVENT& type) const {
	return (handlers.count(type) > 0);
}
