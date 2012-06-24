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

#include "event.h"
#include "engine.h"

event::event(engine* e_) : thread_base("event"), e(e_),
user_queue_lock()
{
	AtomicSet(&handlers_lock, 0);
	
	mouse_down_state[0] = mouse_down_state[1] = mouse_down_state[2] =
	mouse_up_state[0] = mouse_up_state[1] = mouse_up_state[2] =
		ipnt(-1, -1);
	
	lm_double_click_timer = SDL_GetTicks();
	rm_double_click_timer = SDL_GetTicks();
	mm_double_click_timer = SDL_GetTicks();
	
	ldouble_click_time = 200;
	rdouble_click_time = 200;
	mdouble_click_time = 200;

	shift = false;
	alt = false;
	
	this->start();
}

event::~event() {
}

void event::run() {
	// user events are handled "asynchronously", so they don't
	// interfere with other (internal) events or engine code
	
	// copy/move user events to a processing queue, so the queue can
	// still be used in the other (main) event thread
	user_queue_lock.lock();
	user_event_queue_processing.swap(user_event_queue);
	user_queue_lock.unlock();
	
	handle_user_events();
}

/*! handles the sdl events
 */
void event::handle_events() {
	// always acquire the gl context for internal handlers, since these are very likely to modify gl data
	e->acquire_gl_context();
	
	// internal engine event handler
	while(SDL_PollEvent(&event_handle)) {
		const unsigned int event_type = event_handle.type;
		const unsigned int cur_ticks = SDL_GetTicks();
		
		if(event_type == SDL_MOUSEBUTTONDOWN ||
		   event_type == SDL_MOUSEBUTTONUP) {
			// mouse event handling
			const int2 mouse_coord = int2(event_handle.button.x, event_handle.button.y);
			
			switch(event_type) {
				case SDL_MOUSEBUTTONDOWN: {
					switch(event_handle.button.button) {
						case SDL_BUTTON_LEFT:
							if(event_handle.button.state == SDL_PRESSED) {
								mouse_down_state[0] = mouse_coord;
								handle_event(EVENT_TYPE::MOUSE_LEFT_DOWN,
											 make_shared<mouse_left_down_event>(cur_ticks, mouse_coord));
							}
							break;
						case SDL_BUTTON_RIGHT:
							if(event_handle.button.state == SDL_PRESSED) {
								mouse_down_state[1] = mouse_coord;
								handle_event(EVENT_TYPE::MOUSE_RIGHT_DOWN,
											 make_shared<mouse_right_down_event>(cur_ticks, mouse_coord));
							}
							break;
						case SDL_BUTTON_MIDDLE:
							if(event_handle.button.state == SDL_PRESSED) {
								mouse_down_state[2] = mouse_coord;
								handle_event(EVENT_TYPE::MOUSE_MIDDLE_DOWN,
											 make_shared<mouse_middle_down_event>(cur_ticks, mouse_coord));
							}
							break;
					}
				}
				break;
				case SDL_MOUSEBUTTONUP: {
					switch(event_handle.button.button) {
						case SDL_BUTTON_LEFT:
							if(event_handle.button.state == SDL_RELEASED) {
								mouse_up_state[0] = mouse_coord;
								handle_event(EVENT_TYPE::MOUSE_LEFT_UP,
											 make_shared<mouse_left_up_event>(cur_ticks, mouse_coord));
								
								if(cur_ticks - lm_double_click_timer < ldouble_click_time) {
									// emit a double click event
									handle_event(EVENT_TYPE::MOUSE_LEFT_DOUBLE_CLICK,
												 make_shared<mouse_left_double_click_event>(
													cur_ticks,
													prev_events[EVENT_TYPE::MOUSE_LEFT_DOWN],
													prev_events[EVENT_TYPE::MOUSE_LEFT_UP]));
								}
								else {
									// only emit a normal click event
									handle_event(EVENT_TYPE::MOUSE_LEFT_CLICK,
												 make_shared<mouse_left_click_event>(
													cur_ticks,
													prev_events[EVENT_TYPE::MOUSE_LEFT_DOWN],
													prev_events[EVENT_TYPE::MOUSE_LEFT_UP]));
								}
								
								lm_double_click_timer = cur_ticks;
							}
							break;
						case SDL_BUTTON_RIGHT:
							if(event_handle.button.state == SDL_RELEASED) {
								mouse_up_state[1] = mouse_coord;
								handle_event(EVENT_TYPE::MOUSE_RIGHT_UP,
											 make_shared<mouse_right_up_event>(cur_ticks, mouse_coord));
								
								if(cur_ticks - rm_double_click_timer < rdouble_click_time) {
									// emit a double click event
									handle_event(EVENT_TYPE::MOUSE_RIGHT_DOUBLE_CLICK,
												 make_shared<mouse_right_double_click_event>(
													cur_ticks,
													prev_events[EVENT_TYPE::MOUSE_RIGHT_DOWN],
													prev_events[EVENT_TYPE::MOUSE_RIGHT_UP]));
								}
								else {
									// only emit a normal click event
									handle_event(EVENT_TYPE::MOUSE_RIGHT_CLICK,
												 make_shared<mouse_right_click_event>(
													cur_ticks,
													prev_events[EVENT_TYPE::MOUSE_RIGHT_DOWN],
													prev_events[EVENT_TYPE::MOUSE_RIGHT_UP]));
								}
								
								rm_double_click_timer = cur_ticks;
							}
							break;
						case SDL_BUTTON_MIDDLE:
							if(event_handle.button.state == SDL_RELEASED) {
								mouse_up_state[2] = mouse_coord;
								handle_event(EVENT_TYPE::MOUSE_MIDDLE_UP,
											 make_shared<mouse_middle_up_event>(cur_ticks, mouse_coord));
								
								if(SDL_GetTicks() - mm_double_click_timer < mdouble_click_time) {
									// emit a double click event
									handle_event(EVENT_TYPE::MOUSE_MIDDLE_DOUBLE_CLICK,
												 make_shared<mouse_middle_double_click_event>(
													cur_ticks,
													prev_events[EVENT_TYPE::MOUSE_MIDDLE_DOWN],
													prev_events[EVENT_TYPE::MOUSE_MIDDLE_UP]));
								}
								else {
									// only emit a normal click event
									handle_event(EVENT_TYPE::MOUSE_MIDDLE_CLICK,
												 make_shared<mouse_middle_click_event>(
													cur_ticks,
													prev_events[EVENT_TYPE::MOUSE_MIDDLE_DOWN],
													prev_events[EVENT_TYPE::MOUSE_MIDDLE_UP]));
								}
								
								mm_double_click_timer = cur_ticks;
							}
							break;
					}
				}
				break;
			}
		}
		else if(event_type == SDL_MOUSEMOTION ||
				event_type == SDL_MOUSEWHEEL) {
			switch (event_type) {
				case SDL_MOUSEMOTION: {
					const int2 abs_pos = int2(event_handle.motion.x, event_handle.motion.y);
					const int2 rel_move = int2(event_handle.motion.xrel, event_handle.motion.yrel);
					handle_event(EVENT_TYPE::MOUSE_MOVE,
								 make_shared<mouse_move_event>(cur_ticks, abs_pos, rel_move));
				}
				break;
				case SDL_MOUSEWHEEL:
					if(event_handle.wheel.y > 0) {
						handle_event(EVENT_TYPE::MOUSE_WHEEL_UP,
									 make_shared<mouse_wheel_up_event>(
										cur_ticks,
										event_handle.wheel.y));
					}
					else if(event_handle.wheel.y < 0) {
						const unsigned int abs_wheel_move = (unsigned int)abs(event_handle.wheel.y);
						handle_event(EVENT_TYPE::MOUSE_WHEEL_DOWN,
									 make_shared<mouse_wheel_down_event>(
										cur_ticks,
										abs_wheel_move));
					}
					break;
			}
		}
		else {
			// key, etc. event handling
			switch(event_type) {
				case SDL_KEYUP:
					handle_event(EVENT_TYPE::KEY_UP,
								 make_shared<key_up_event>(cur_ticks, event_handle.key.keysym.sym));
					break;
				case SDL_KEYDOWN:
					handle_event(EVENT_TYPE::KEY_DOWN,
								 make_shared<key_up_event>(cur_ticks, event_handle.key.keysym.sym));
					break;
				case SDL_TEXTINPUT: {
					string text = "";
					for(size_t i = 0; i < SDL_TEXTINPUTEVENT_TEXT_SIZE; i++) {
						if(event_handle.text.text[i] == 0) break;
						text += event_handle.text.text[i];
					}
					const vector<unsigned int> codes(unicode::utf8_to_unicode(text));
					for(const auto& code : codes) {
						handle_event(EVENT_TYPE::UNICODE_INPUT,
									 make_shared<unicode_input_event>(cur_ticks, code));
					}
				}
				break;
				case SDL_WINDOWEVENT:
					if(event_handle.window.event == SDL_WINDOWEVENT_RESIZED) {
						const size2 new_size(event_handle.window.data1, event_handle.window.data2);
						handle_event(EVENT_TYPE::WINDOW_RESIZE,
									 make_shared<window_resize_event>(cur_ticks, new_size));
					}
					break;
				case SDL_QUIT:
					handle_event(EVENT_TYPE::QUIT, make_shared<quit_event>(cur_ticks));
					break;
				default: break;
			}
		}
	}
	
	e->release_gl_context();
}

/*! gets the mouses position (pnt)
 */
pnt event::get_mouse_pos() const {
	pnt pos;
	SDL_GetMouseState((int*)&pos.x, (int*)&pos.y);
	return pos;
}

void event::set_ldouble_click_time(unsigned int dctime) {
	ldouble_click_time = dctime;
}

void event::set_rdouble_click_time(unsigned int dctime) {
	rdouble_click_time = dctime;
}

void event::set_mdouble_click_time(unsigned int dctime) {
	mdouble_click_time = dctime;
}

void event::add_event_handler(handler& handler_, EVENT_TYPE type) {
	while(!AtomicCAS(&handlers_lock, 0, handlers_locked)) {
		this_thread::yield();
	}
	handlers.insert(pair<EVENT_TYPE, handler&>(type, handler_));
	AtomicSet(&handlers_lock, 0);
}

void event::add_internal_event_handler(handler& handler_, EVENT_TYPE type) {
	while(!AtomicCAS(&handlers_lock, 0, handlers_locked)) {
		this_thread::yield();
	}
	internal_handlers.insert(pair<EVENT_TYPE, handler&>(type, handler_));
	AtomicSet(&handlers_lock, 0);
}

void event::add_event(const EVENT_TYPE type, shared_ptr<event_object> obj) {
	// for now, just pass it through
	handle_event(type, obj);
}

void event::handle_event(const EVENT_TYPE& type, shared_ptr<event_object> obj) {
	// set/override last event for this type
	prev_events[type] = obj;
	
	// call internal event handlers directly
	int cur_hl = AtomicGet(&handlers_lock);
	while(cur_hl == handlers_locked ||
		  !AtomicCAS(&handlers_lock, cur_hl, cur_hl+1)) {
		this_thread::yield();
		cur_hl = AtomicGet(&handlers_lock);
	}
	
	const auto range = internal_handlers.equal_range(type);
	for(auto iter = range.first; iter != range.second; iter++) {
		// ignore return value for now (TODO: actually use this?)
		iter->second(type, obj);
	}
	
	AtomicFetchThenDecrement(&handlers_lock);
	
	// push to user event queue (these will be handled later on)
	user_queue_lock.lock();
	user_event_queue.push(make_pair(type, obj));
	user_queue_lock.unlock();
}

void event::handle_user_events() {
	while(!user_event_queue_processing.empty()) {
		// pop next event
		pair<EVENT_TYPE, shared_ptr<event_object>> evt = user_event_queue_processing.front();
		user_event_queue_processing.pop();
		
		// call user event handlers
		int cur_hl = AtomicGet(&handlers_lock);
		while(cur_hl == handlers_locked ||
			  !AtomicCAS(&handlers_lock, cur_hl, cur_hl+1)) {
			this_thread::yield();
			cur_hl = AtomicGet(&handlers_lock);
		}
		
		const auto range = handlers.equal_range(evt.first);
		for(auto iter = range.first; iter != range.second; iter++) {
			iter->second(evt.first, evt.second);
		}
		
		AtomicFetchThenDecrement(&handlers_lock);
	}
}

void event::remove_event_handler(const handler& handler_) {
	while(!AtomicCAS(&handlers_lock, 0, handlers_locked)) {
		this_thread::yield();
	}
	
	for(auto handler_iter = handlers.cbegin(); handler_iter != handlers.cend(); ) {
		// good old pointer comparison ...
		if(&handler_iter->second == &handler_) {
			handler_iter = handlers.erase(handler_iter);
		}
		else ++handler_iter;
	}
	for(auto handler_iter = internal_handlers.cbegin(); handler_iter != internal_handlers.cend(); ) {
		if(&handler_iter->second == &handler_) {
			handler_iter = internal_handlers.erase(handler_iter);
		}
		else ++handler_iter;
	}
	
	AtomicSet(&handlers_lock, 0);
}

void event::remove_event_types_from_handler(const handler& handler_, const set<EVENT_TYPE>& types) {
	while(!AtomicCAS(&handlers_lock, 0, handlers_locked)) {
		this_thread::yield();
	}
	
	for(const auto& type : types) {
		const auto range_0 = handlers.equal_range(type);
		handlers.erase(range_0.first, range_0.second);
		const auto range_1 = internal_handlers.equal_range(type);
		internal_handlers.erase(range_1.first, range_1.second);
	}
	
	AtomicSet(&handlers_lock, 0);
}
