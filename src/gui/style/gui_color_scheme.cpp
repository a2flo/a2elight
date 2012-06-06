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

#include "gui_color_scheme.h"
#include "engine.h"

#define A2E_COLOR_SCHEME_VERSION 1

gui_color_scheme::gui_color_scheme(engine* e_) : e(e_), x(e->get_xml()) {
}

gui_color_scheme::~gui_color_scheme() {
}

bool gui_color_scheme::load(const string& filename) {
	xml::xml_doc ui_doc = x->process_file(e->data_path(filename), false); // TODO: DTD!
	if(!ui_doc.valid) {
		a2e_error("couldn't process color scheme file %s!", filename);
		return false;
	}
	
	const size_t doc_version = ui_doc.get<size_t>("a2e_color_scheme.version");
	if(doc_version != A2E_COLOR_SCHEME_VERSION) {
		a2e_error("invalid color scheme version: %u (should be %u)!",
				  doc_version, A2E_COLOR_SCHEME_VERSION);
		return false;
	}
	
	// process nodes
	for(const auto& node : ui_doc.nodes) {
		process_node(node.second, nullptr);
	}
	
	return true;
}

void gui_color_scheme::process_node(const xml::xml_node* node, const xml::xml_node* parent) {
	// process node itself
	const string name = (*node)["name"];
	const string color = (*node)["value"];
	if(name == "INVALID" || color == "INVALID") {
		a2e_error("incomplete color definition");
		return;
	}
	if(colors.count(name) > 0) {
		a2e_error("a color definition with such a name (%s) already exists!", name);
		return;
	}
	
	// there are two possibilities to specify a color:
	if(color.find(',') != string::npos) {
		// first: a float4 color (RGBA)
		const vector<string> tokens = core::tokenize(color, ',');
		if(tokens.size() != 4) {
			a2e_error("invalid float4 color: %s", color);
			return;
		}
		colors.insert(make_pair(name, float4(strtof(tokens[0].c_str(), nullptr),
											 strtof(tokens[1].c_str(), nullptr),
											 strtof(tokens[2].c_str(), nullptr),
											 strtof(tokens[3].c_str(), nullptr))));
	}
	else {
		// second: an unsinged int/hex color (ARGB)
		const unsigned int conv_color = string2uint("0x"+color);
		colors.insert(make_pair(name, float4((conv_color >> 16) & 0xFF,
											 (conv_color >> 8) & 0xFF,
											 conv_color & 0xFF,
											 0xFF - ((conv_color >> 24) & 0xFF)) / 255.0f));
	}
	
	// process child nodes
	for(const auto& child : node->children) {
		process_node(child.second, node);
	}
}

const float4 gui_color_scheme::get(const string& name) const {
	static const float4 invalid_color(0.0f, 0.0f, 0.0f, 0.0f);
	const auto color = colors.find(name);
	if(color == colors.end()) return invalid_color;
	return color->second;
}
