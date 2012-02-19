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

#include "gui_object.h"
#include "engine.h"
#include "core/core.h"

gui_object::gui_object(engine* e_) :
e(e_), g(e_->get_gfx()), rect(), parent(nullptr) {
	//
	state.visible = true;
	state.enabled = true;
}

gui_object::~gui_object() {
	//a2e_debug("deleting gui_object (id #%i, type: %s) object", id, type_str.c_str());
	
	// delete all event callbacks for this object
	//evt->delete_event_callbacks(id);
	
	a2e_debug("gui_object object deleted");
}

