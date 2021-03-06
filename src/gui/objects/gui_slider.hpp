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

#ifndef __A2E_GUI_SLIDER_HPP__
#define __A2E_GUI_SLIDER_HPP__

#include "gui/objects/gui_object.hpp"

class gui_slider : public gui_object {
public:
	using gui_object::gui_object;
	virtual ~gui_slider() = default;
	
	virtual void draw();
	virtual void compute_abs_values();
	
	//
	void set_knob_position(const float& pos);
	float get_knob_position() const;
	
	//
	virtual bool should_handle_mouse_event(const EVENT_TYPE& type, const int2& point) const;
	virtual bool handle_mouse_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj, const int2& point);
	
protected:
	float knob_radius { gui_theme::point_to_pixel(6.0f) }; // in pt
	float knob_offset { gui_theme::point_to_pixel(knob_radius + 2.0f) }; // in pt
	atomic<float> knob_position { 0.5f };
	float slider_width { 0.0f };
	
	bool move_knob(const int2& point);

};

#endif
