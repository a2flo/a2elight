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

#ifndef __GUI_COLOR_THEME_H__
#define __GUI_COLOR_THEME_H__

#include "global.h"
#include "core/xml.h"
#include "gui_color_scheme.h"

class engine;
class xml;
class gfx;
class gui_theme {
public:
	gui_theme(engine* e);
	~gui_theme();
	
	bool load(const string& filename);
	
protected:
	engine* e;
	xml* x;
	gui_color_scheme scheme;
	
	bool load_ui_object(const string& filename);
	
	void process_state(const xml::xml_node* node, const xml::xml_node* parent);
	void process_primitive(const xml::xml_node* node, const xml::xml_node* parent);
	
	//
	typedef std::function<void(const pnt& offset, const size2& size)> gui_ui_object_draw_function;
	struct gui_ui_object {
		vector<gui_ui_object_draw_function> draw_calls;
	};
	
};

#endif
