/*
 *  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2013 Florian Ziesche
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

#ifndef __A2E_GUI_POP_UP_BUTTON_HPP__
#define __A2E_GUI_POP_UP_BUTTON_HPP__

#include "gui/objects/gui_text.hpp"
#include "gui/objects/gui_item_container.hpp"

class gui_pop_up_window;
class A2E_API gui_pop_up_button : public gui_item_container {
public:
	gui_pop_up_button(engine* e, const float2& size, const float2& position);
	virtual ~gui_pop_up_button();
	
	virtual void draw();
	
	virtual bool handle_mouse_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj, const ipnt& point);

	virtual void set_active(const bool& state);
	
	using gui_object::redraw;
	
protected:
	gui_pop_up_window* selection_wnd = nullptr;
	void open_selection_wnd();
	void close_selection_wnd();
	
	using gui_object::handle;

};

#endif
