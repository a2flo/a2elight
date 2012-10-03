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

#ifndef __A2E_GUI_SLIDER_H__
#define __A2E_GUI_SLIDER_H__

#include "gui/objects/gui_object.h"

class A2E_API gui_slider : public gui_object {
public:
	gui_slider(engine* e, const float2& size, const float2& position);
	virtual ~gui_slider();
	
	virtual void draw();
	virtual void compute_abs_values();
	
	//
	void set_knob_position(const float& pos);
	float get_knob_position() const;
	
	//
	virtual bool should_handle_mouse_event(const EVENT_TYPE& type, const ipnt& point) const;
	virtual bool handle_mouse_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj, const ipnt& point);
	
protected:
	const float knob_radius = 16.0f; // TODO: dpi?
	const float knob_offset = knob_radius + 4.0f; // TODO: dpi?
	atomic<float> knob_position { 0.5f };
	float slider_width = 0.0f;
	
	bool move_knob(const ipnt& point);

};

#endif
