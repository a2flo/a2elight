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

#ifndef __A2E_GUI_WINDOW_HPP__
#define __A2E_GUI_WINDOW_HPP__

#include "gui/objects/gui_object.hpp"
#include "gui/style/gui_surface.hpp"

class gui_window : public gui_object, public gui_surface {
public:
	gui_window(const float2& buffer_size, const float2& position, const SURFACE_FLAGS flags = SURFACE_FLAGS::NONE);
	virtual ~gui_window();
	
	void draw() override;
	
	// takes care of both gui_object and gui_surface functions,
	// which serve the same purpose in case of gui_window
	void redraw() override;
	bool needs_redraw() const override;
	
	void resize(const float2& buffer_size) override; // from gui_surface
	void set_size(const float2& size) override; // from gui_object
	void set_position(const float2& position) override; // from gui_object
	
	int2 abs_to_rel_position(const int2& point) const override;
	int2 rel_to_abs_position(const int2& point) const override;
	
	bool handle_mouse_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj, const int2& point) override;
	bool handle_key_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj) override;
	
	void clear(const bool delete_children = true);
	
	//! sets the window background color (the default is a fully transparent background)
	virtual void set_background_color(const float4& color);
	virtual const float4& get_background_color() const;

protected:
	float4 background_color { 0.0f, 0.0f, 0.0f, 0.0f };
	
	virtual bool is_window() const override { return true; }

};

#endif
