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

#ifndef __A2E_GUI_POP_UP_BUTTON_H__
#define __A2E_GUI_POP_UP_BUTTON_H__

#include "gui/objects/gui_text.h"

class gui_pop_up_window;
class A2E_API gui_pop_up_button : public gui_object {
public:
	gui_pop_up_button(engine* e, const float2& size, const float2& position);
	virtual ~gui_pop_up_button();
	
	virtual void draw();
	
	virtual bool handle_mouse_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj, const ipnt& point);
	
	virtual void clear();
	virtual void add_item(const string& identifier, const string& label);
	virtual void remove_item(const string& identifier);
	
	virtual const pair<const string, string>* get_selected_item() const;
	virtual void set_selected_item(const string& identifier, const bool event_on_equal = false);
	virtual void set_selected_item(const size_t& index, const bool event_on_equal = false);
	
	virtual void set_active(const bool& state);
	
protected:
	// <identifier, label>
	unordered_map<string, string> items;
	vector<typename decltype(items)::value_type*> display_items;
	typename decltype(items)::value_type* selected_item = nullptr;
	
	gui_pop_up_window* selection_wnd = nullptr;
	void open_selection_wnd();
	void close_selection_wnd();

};

#endif
