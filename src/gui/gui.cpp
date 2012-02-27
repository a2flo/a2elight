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

#include "gui.h"
#include "engine.h"

gui::gui(engine* e_) :
thread_base("gui"),
e(e_), evt(e_->get_event()), g(e_->get_gfx()),
key_handler_fnctr(this, &gui::key_handler), mouse_handler_fnctr(this, &gui::mouse_handler) {
	AtomicSet(&keyboard_input, 1);
	AtomicSet(&mouse_input, 1);

	// create keyboard/mouse event handlers
	evt->add_event_handler(key_handler_fnctr,
						   EVENT_TYPE::KEY_DOWN,
						   EVENT_TYPE::KEY_UP,
						   EVENT_TYPE::KEY_PRESSED);
	evt->add_event_handler(mouse_handler_fnctr,
						   EVENT_TYPE::MOUSE_LEFT_DOWN,
						   EVENT_TYPE::MOUSE_LEFT_UP,
						   EVENT_TYPE::MOUSE_LEFT_CLICK,
						   EVENT_TYPE::MOUSE_LEFT_DOUBLE_CLICK,
						   EVENT_TYPE::MOUSE_LEFT_HOLD,
						   
						   EVENT_TYPE::MOUSE_RIGHT_DOWN,
						   EVENT_TYPE::MOUSE_RIGHT_UP,
						   EVENT_TYPE::MOUSE_RIGHT_CLICK,
						   EVENT_TYPE::MOUSE_RIGHT_DOUBLE_CLICK,
						   EVENT_TYPE::MOUSE_RIGHT_HOLD,
						   
						   EVENT_TYPE::MOUSE_MIDDLE_DOWN,
						   EVENT_TYPE::MOUSE_MIDDLE_UP,
						   EVENT_TYPE::MOUSE_MIDDLE_CLICK,
						   EVENT_TYPE::MOUSE_MIDDLE_DOUBLE_CLICK,
						   EVENT_TYPE::MOUSE_MIDDLE_HOLD,
						   
						   EVENT_TYPE::MOUSE_MOVE,
						   
						   EVENT_TYPE::MOUSE_WHEEL_UP,
						   EVENT_TYPE::MOUSE_WHEEL_DOWN);
	
	// start gui thread
	this->start();
}

gui::~gui() {
	a2e_debug("deleting gui object");
	
	evt->remove_event_handler(key_handler_fnctr);
	evt->remove_event_handler(mouse_handler_fnctr);

	a2e_debug("gui object deleted");
}

void gui::init() {
	// start actual gui thread when everything is initialized
	this->start();
}

void gui::run() {
}

void gui::draw() {
	// draw windows
}

bool gui::key_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(!get_keyboard_input()) return false;
	return false;
}

bool gui::mouse_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(!get_mouse_input()) return false;
	//cout << "mouse event: " << (unsigned int)type << endl;
	return false;
}

void gui::set_keyboard_input(const bool& state) {
	AtomicSet(&keyboard_input, state ? 1 : 0);
}

void gui::set_mouse_input(const bool& state) {
	AtomicSet(&mouse_input, state ? 1 : 0);
}

bool gui::get_keyboard_input() const {
	const int state = AtomicGet((atomic_t*)&keyboard_input);
	return (state == 0 ? false : true);
}

bool gui::get_mouse_input() const {
	const int state = AtomicGet((atomic_t*)&mouse_input);
	return (state == 0 ? false : true);
}
