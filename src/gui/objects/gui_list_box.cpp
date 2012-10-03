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

#include "gui_list_box.h"
#include "engine.h"
#include "gui.h"

gui_list_box::gui_list_box(engine* e_, const float2& size_, const float2& position_) :
gui_object(e_, size_, position_) {
	//
}

gui_list_box::~gui_list_box() {
}

void gui_list_box::draw() {
	if(!gui_object::handle_draw()) return;
	
	// TODO: handle disabled state
	theme->draw("list_box", "normal", position_abs, size_abs);
	// TODO: !
}

bool gui_list_box::handle_mouse_event(const EVENT_TYPE& type a2e_unused, const shared_ptr<event_object>& obj a2e_unused, const ipnt& point a2e_unused) {
	if(!state.visible || !state.enabled) return false;
	// TODO: !
	return false;
}

bool gui_list_box::should_handle_mouse_event(const EVENT_TYPE& type, const ipnt& point) const {
	return gui_object::should_handle_mouse_event(type, point);
}
