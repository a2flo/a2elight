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
#include "rendering/rtt.h"

/*! @class gui
 *  @brief graphical user interface functions
 */

class engine;
class core;
class scene;
class shader;
class shader_gl3;
typedef shared_ptr<shader_gl3> gl3shader;

class A2E_API gui : public thread_base {
public:
	gui(engine* e);
	~gui();

	void draw();
	
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
	
	// misc flags
	void set_keyboard_input(const bool& state);
	bool get_keyboard_input() const;
	void set_mouse_input(const bool& state);
	bool get_mouse_input() const;
	
	// draw callbacks
	enum class DRAW_MODE_UI : unsigned int {
		PRE_UI,
		POST_UI
	};
	typedef functor<void, const gui::DRAW_MODE_UI> draw_callback;
	void add_draw_callback(draw_callback& cb);
	void delete_draw_callback(draw_callback& cb);

protected:
	engine* e;
	event* evt;
	rtt* r;
	shader* s;
	scene* sce;
	
	// note: this must be ordered
	vector<draw_callback*> draw_callbacks;
	
	rtt::fbo* main_fbo;
	void recreate_buffers(const size2 size);
	void delete_buffers();
	
	void reload_shaders();
	gl3shader blend_shd;
	
	virtual void run();
	
	// event handling
	event::handler key_handler_fnctr;
	event::handler mouse_handler_fnctr;
	event::handler shader_reload_fnctr;
	event::handler window_handler_fnctr;
	bool key_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	bool mouse_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	bool shader_reload_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	bool window_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	
	atomic_t keyboard_input;
	atomic_t mouse_input;

};

#endif
