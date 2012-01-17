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

#ifndef __EVENT_H__
#define __EVENT_H__

#include "global.h"

#include "core/core.h"
#include "event_objects.h"
#include "threading/thread_base.h"

/*! @class event
 *  @brief (sdl) event handler
 */

class A2E_API event : public thread_base {
public:
	event();
	virtual ~event();

	void handle_events();
	
	// <returns true if handled, pointer to object, event type>
	typedef functor<bool, EVENT_TYPE, shared_ptr<event_object>> handler;
	void add_event_handler(handler& handler_, EVENT_TYPE type);
	template<typename... event_types> void add_event_handler(handler& handler_, event_types&&... types) {
		// unwind types, always call the simple add handler for each type
		unwind_add_event_handler(handler_, std::forward<event_types>(types)...);
	}

	//! input types
	/*enum INPUT_TYPE {
		IT_LEFT,	//!< enum left arrow key
		IT_RIGHT,	//!< enum right arrow key
		IT_BACK,	//!< enum backspace key
		IT_DEL,		//!< enum delete key
		IT_HOME,	//!< enum home key
		IT_END		//!< enum end key
	};*/

	//! gets the mouses position (pnt)
	pnt get_mouse_pos() const;
	
	void set_ldouble_click_time(unsigned int dctime);
	void set_rdouble_click_time(unsigned int dctime);
	void set_mdouble_click_time(unsigned int dctime);

protected:
	SDL_Event event_handle;
	
	virtual void run();
	
	//
	unordered_multimap<EVENT_TYPE, handler&> internal_handlers;
	unordered_multimap<EVENT_TYPE, handler&> handlers;
	queue<pair<EVENT_TYPE, shared_ptr<event_object>>> user_event_queue, user_event_queue_processing;
#if !defined(GCC_LEGACY)
	recursive_mutex user_queue_lock;
#else
	SDL_mutex* user_queue_lock;
#endif
	void handle_event(const EVENT_TYPE& type, shared_ptr<event_object> obj);
	void handle_user_events();
	
	//
	unordered_map<EVENT_TYPE, shared_ptr<event_object>> prev_events;
	
	// [left, right, middle]
	ipnt mouse_down_state[3];
	ipnt mouse_up_state[3];
	
	//! timer that decides if there is a * mouse double click
	unsigned int lm_double_click_timer;
	unsigned int rm_double_click_timer;
	unsigned int mm_double_click_timer;
	
	//! config setting for * mouse double click "timeframe"
	unsigned int ldouble_click_time;
	unsigned int rdouble_click_time;
	unsigned int mdouble_click_time;
	
	// key states
	bool shift;
	bool alt;
	
	//
	void unwind_add_event_handler(handler& handler_, EVENT_TYPE type) {
		add_event_handler(handler_, type);
	}
	template<typename... event_types> void unwind_add_event_handler(handler& handler_, EVENT_TYPE type, event_types&&... types) {
		// unwind types, always call the simple add handler for each type
		add_event_handler(handler_, type);
		unwind_add_event_handler(handler_, std::forward<event_types>(types)...);
	}

};

#endif
