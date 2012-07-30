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

#ifndef __A2E_GUI_OBJECT_H__
#define __A2E_GUI_OBJECT_H__

#include "global.h"
#include "core/vector2.h"

/*! @class gui_object
 *  @brief gui object element functions
 */

class engine;
class core;

class A2E_API gui_object {
public:
	gui_object(engine* e_);
	virtual ~gui_object();
	
/*	//! gui object types - same as in the gui class
	GUI_OBJECT_TYPE {
		GUI_NONE,		//!< enum none/dummy type
		GUI_BUTTON,		//!< enum button type
		GUI_INPUT,		//!< enum input box type
		GUI_TEXT,		//!< enum static text type
		GUI_LIST,		//!< enum list box type
		GUI_HBAR,		//!< enum horizontal bar type
		GUI_VBAR,		//!< enum vertical bar type
		GUI_CHECK,		//!< enum check box type
		GUI_COMBO,		//!< enum combo box type
		GUI_WINDOW,		//!< enum window type
		GUI_MLTEXT,		//!< enum multi line text type
		GUI_IMAGE,		//!< enum image type
		GUI_TAB,		//!< enum tab type
		GUI_TOGGLE,		//!< enum toggle button type
		GUI_OPENDIALOG,	//!< enum open file dialog type
		GUI_SAVEDIALOG,	//!< enum save file dialog type
		GUI_MSGBOX_OK	//!< enum message box type
	};*/

protected:
	engine* e;
	
	// gui object element variables
	struct {
		bool visible;
		bool enabled;
	} state;
	
	rect rectangle;
	
	gui_object* parent;
	
	/*gfx* g;
	unicode* u;
	gui_helper<gui_object>* gh;
	gui_style* gs;
	event* evt;
	a2eui* ui;

	// gui object element variables

	//! the object's id
	GUI_ID id;
	//! the object's rectangle
	gfx::rect* rectangle;
	//! the object's state
	string state;
	//! the object's type (enum)
	gui_object::GUI_OBJECT_TYPE type;
	//! the object's type (name)
	string type_str;
	//! the object's text handler
	text* text_handler;
	//! the object's text
	string textstr;
	//! the object's image
	image* img;
	//! visible flag
	bool visible;
	//! redraw flag
	bool redraw;
	//! scissor flag
	bool scissor;
	//! sub-object flag (if the object is contained within another object)
	bool sub_object;
	//! the object's tab
	unsigned int selected_tab;
	//! the object's tab_id
	GUI_ID tab_id;
	//! the object's window_id
	GUI_ID window_id;
	//! the object's (parent) tab pointer
	gui_object* tab;
	//! the object's (parent) window pointer
	gui_object* window;
	//! the current offset (screen coordinate)
	ipnt cur_offset;
	
	//! some tmp variables
	ipnt p;
	ipnt p2;
	gfx::rect r1;
	gfx::rect r2;
	
	//! the objects child objects
	deque<gui_object*> childs;


	// id handler
	unsigned int get_free_id();
	bool id_in_use(unsigned int id);
	void remove_id(unsigned int id);
	void set_last_id(unsigned int id);
	void clear_ids();
	set<unsigned int> used_ids;
	unsigned int last_id;*/

};

#endif
