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

#include "a2eui.h"
#include "engine.h"

#define A2E_UI_VERSION 3

a2eui::a2eui(engine* e_) : e(e_), x(e->get_xml()) {
}

a2eui::~a2eui() {
}

void a2eui::load(const string& filename) {
	xml::xml_doc ui_doc = x->process_file(e->data_path(filename));
	if(!ui_doc.valid) {
		a2e_error("couldn't process ui file %s!", filename);
		return;
	}
	
	const size_t doc_version = ui_doc.get<size_t>("a2e_user_interface.version");
	if(doc_version != A2E_UI_VERSION) {
		a2e_error("invalid a2eui version: %u (should be %u)!", doc_version, A2E_UI_VERSION);
		return;
	}
	
	// process nodes
	for(const auto& node : ui_doc.nodes) {
		process_node(node.second, nullptr);
	}
}

void a2eui::process_node(const xml::xml_node* node, const xml::xml_node* parent) {
	// process child nodes
	for(const auto& child : node->children) {
		process_node(child.second, node);
	}
}
