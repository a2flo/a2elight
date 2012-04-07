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

#ifndef __GUI_H__
#define __GUI_H__

#include "global.h"
#include "threading/thread_base.h"
#include "gui/event.h"

/*! @class gui
 *  @brief graphical user interface functions
 */

class engine;
class core;
class gfx;

class A2E_API gui : public thread_base {
public:
	gui(engine* e);
	~gui();
	
	// gui event types
	enum class GUI_EVENT_TYPE : unsigned int {
		// TODO: form: present! (also: string or enum?)
		
		BUTTON_PRESS,
		BUTTON_RIGHT_PRESS,
		//TOGGLE_BUTTON_PRESS,
		
		LIST_ITEM_PRESS,
		LIST_ITEM_DOUBLE_CLICK,
		
		CHECKBOX_TOGGLE,
		
		RADIO_PRESS,
		
		COMBO_ITEM_PRESS,
		
		INPUT_SELECT,
		INPUT_UNSELECT,
		
		BAR_SCROLL,
		
		FILE_OPEN,
		FILE_SAVE,
		
		SLIDER_MOVE,
		
		// TODO: tree list
		// TODO: color picker
		// TODO: progress bar
		// TODO: date picker
		
		WINDOW_CLOSE,
		WINDOW_OPEN,
		
		TAB_SELECT,
	};

	void init();
	void draw();
	
	void set_keyboard_input(const bool& state);
	bool get_keyboard_input() const;
	void set_mouse_input(const bool& state);
	bool get_mouse_input() const;

protected:
	engine* e;
	event* evt;
	
	virtual void run();
	
	// event handling
	event::handler key_handler_fnctr;
	event::handler mouse_handler_fnctr;
	bool key_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	bool mouse_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	
	atomic_t keyboard_input;
	atomic_t mouse_input;

};

#endif
