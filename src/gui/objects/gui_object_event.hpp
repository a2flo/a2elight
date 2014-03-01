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

#ifndef __A2E_GUI_OBJECT_EVENT_HPP__
#define __A2E_GUI_OBJECT_EVENT_HPP__

#include "global.hpp"
#include "gui/gui_event.hpp"
#include "core/event.hpp"

class gui;
class A2E_API gui_object_event {
public:
	gui_object_event();
	virtual ~gui_object_event();

	//
	void lock();
	bool try_lock();
	void unlock();
	
	//
	typedef std::function<void(GUI_EVENT, gui_object&)> handler;
	void add_handler(handler&& handler_, GUI_EVENT type);
	template<typename... event_types> void add_handler(handler&& handler_, event_types&&... types) {
		// unwind types, always call the simple add handler for each type
		unwind_add_handler(std::forward<handler>(handler_), std::forward<event_types>(types)...);
	}
	void remove_handlers(const GUI_EVENT& type);
	void remove_handlers();
	bool has_handler(const GUI_EVENT& type) const;
	
	// must return true if event was handled, false if not!
	virtual bool handle_mouse_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj, const ipnt& point);
	virtual bool handle_key_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj);

protected:
	event* evt;
	
	recursive_mutex mutex;
	
	//
	void unwind_add_handler(handler&& handler_, GUI_EVENT type) {
		add_handler(std::forward<handler>(handler_), type);
	}
	template<typename... event_types> void unwind_add_handler(handler&& handler_, GUI_EVENT type, event_types&&... types) {
		// unwind types, always call the simple add handler for each type
		add_handler(std::forward<handler>(handler_), type);
		unwind_add_handler(std::forward<handler>(handler_), std::forward<event_types>(types)...);
	}
	unordered_multimap<GUI_EVENT, handler> handlers;
	virtual void handle(const GUI_EVENT gui_evt);

};

#endif
