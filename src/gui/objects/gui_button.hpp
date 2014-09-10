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

#ifndef __A2E_GUI_BUTTON_HPP__
#define __A2E_GUI_BUTTON_HPP__

#include "gui/objects/gui_text.hpp"

class gui_button : public gui_text {
public:
	using gui_text::gui_text;
	virtual ~gui_button() = default;
	
	virtual void draw();
	
	virtual bool handle_mouse_event(const EVENT_TYPE& type, const shared_ptr<event_object>& obj, const ipnt& point);
	
protected:

};

#endif
