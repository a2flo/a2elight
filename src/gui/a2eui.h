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

#ifndef __A2EUI_H__
#define __A2EUI_H__

#include "global.h"
#include "core/xml.h"

class engine;
class xml;
class a2eui {
public:
	a2eui(engine* e);
	~a2eui();
	
	void load(const string& filename);
	
protected:
	engine* e;
	xml* x;
	
	void process_node(const xml::xml_node* node, const xml::xml_node* parent);
	
};

#endif
