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

#ifndef __A2E_GUI_EVENT_H__
#define __A2E_GUI_EVENT_H__

// gui event types
enum class GUI_EVENT : unsigned int {
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
	
	WINDOW_OPEN,
	WINDOW_CLOSE,
	
	TAB_SELECT,
};
namespace std {
	template <> struct hash<GUI_EVENT> : public hash<unsigned int> {
		size_t operator()(GUI_EVENT type) const throw() {
			return hash<unsigned int>::operator()((unsigned int)type);
		}
	};
}

#endif
