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

#include "gui_theme.h"
#include "engine.h"

#define A2E_THEME_VERSION 1

gui_theme::gui_theme(engine* e_) : e(e_), x(e->get_xml()) {
}

gui_theme::~gui_theme() {
}

void gui_theme::load(const string& filename) {
	xml::xml_doc ui_doc = x->process_file(e->data_path(filename));
	if(!ui_doc.valid) {
		a2e_error("couldn't process theme file %s!", filename);
		return;
	}
	
	const size_t doc_version = ui_doc.get<size_t>("a2e_theme.version");
	if(doc_version != A2E_THEME_VERSION) {
		a2e_error("invalid theme version: %u (should be %u)!",
				  doc_version, A2E_THEME_VERSION);
		return;
	}
	
	// process nodes
	for(const auto node : ui_doc.nodes) {
		process_node(node.second, NULL);
	}
}

void gui_theme::process_node(const xml::xml_node* node, const xml::xml_node* parent) {
	// process node itself
	/*const string name = (*node)["name"];
	const string color = (*node)["value"];
	if(name == "INVALID" || color == "INVALID") {
		a2e_error("incomplete color definition");
		return;
	}*/
	
	// TODO
	
	// process child nodes
	for(const auto child : node->children) {
		process_node(child.second, node);
	}
}
