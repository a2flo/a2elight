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
#define A2E_UI_OBJECT_VERSION 1

gui_theme::gui_theme(engine* e_) : e(e_), x(e->get_xml()), scheme(e_) {
}

gui_theme::~gui_theme() {
}

bool gui_theme::load(const string& filename) {
	xml::xml_doc ui_doc = x->process_file(e->data_path(filename), false); // TODO: DTD!
	if(!ui_doc.valid || filename.rfind("/") == string::npos) {
		a2e_error("couldn't process theme file %s!", filename);
		return false;
	}
	const string theme_path = filename.substr(0, filename.rfind("/") + 1);
	
	const size_t doc_version = ui_doc.get<size_t>("a2e_theme.version");
	if(doc_version != A2E_THEME_VERSION) {
		a2e_error("invalid theme version: %u (should be %u)!",
				  doc_version, A2E_THEME_VERSION);
		return false;
	}
	
	// load color scheme
	const string scheme_filename = ui_doc.get<string>("a2e_theme.colors");
	if(scheme_filename == "INVALID") {
		a2e_error("no color scheme specified in theme \"%s\"!", filename);
		return false;
	}
	if(!scheme.load(theme_path + scheme_filename)) {
		a2e_error("failed to load color scheme \"%s\" for theme \"%s\"!", scheme_filename, filename);
		return false;
	}
	
	//
	const xml::xml_node* theme_node = ui_doc.get_node("a2e_theme");
	if(theme_node == nullptr || theme_node->children.empty()) {
		a2e_error("theme \"%s\" is empty!", filename);
		return false;
	}
	
	// process nodes
	for(const auto& node : theme_node->children) {
		if(node.first == "object") {
			const string& obj_filename = (*node.second)["file"];
			if(obj_filename != "INVALID") {
				load_ui_object(theme_path + obj_filename);
			}
			else {
				a2e_error("filename is missing for object in theme \"%s\"!", filename);
			}
		}
		else {
			a2e_error("unknown node \"%s\" in theme \"%s\"!", node.first, filename);
		}
	}
	
	return true;
}

bool gui_theme::load_ui_object(const string& filename) {
	xml::xml_doc ui_object_doc = x->process_file(e->data_path(filename), false); // TODO: DTD!
	if(!ui_object_doc.valid) {
		a2e_error("couldn't process ui object file %s!", filename);
		return false;
	}
	
	const size_t obj_version = ui_object_doc.get<size_t>("a2e_ui_object.version");
	if(obj_version != A2E_UI_OBJECT_VERSION) {
		a2e_error("invalid ui object version: %u (should be %u)!",
				  obj_version, A2E_UI_OBJECT_VERSION);
		return false;
	}
	
	//
	const xml::xml_node* obj_node = ui_object_doc.get_node("a2e_ui_object");
	if(obj_node == nullptr || obj_node->children.empty()) {
		a2e_error("ui object \"%s\" is empty!", filename);
		return false;
	}
	
	// process nodes
	for(const auto& node : obj_node->children) {
		if(node.first == "state") {
			process_state(node.second, nullptr);
		}
		else {
			a2e_error("unknown node \"%s\" in ui object \"%s\"!", node.first, filename);
		}
	}
	
	return true;
}

void gui_theme::process_state(const xml::xml_node* node, const xml::xml_node* parent a2e_unused) {
	// process child nodes
	for(const auto& child : node->children) {
		process_primitive(child.second, node);
	}
}

void gui_theme::process_primitive(const xml::xml_node* node a2e_unused, const xml::xml_node* parent a2e_unused) {
	/*cout << "primitive: " << node->name() << endl;
	
	//
	const string color_str = (*node)["color"];
	if(color_str == "INVALID") {
		a2e_error("no color specified for primitive: %s", node->name());
		return;
	}
	const float4 color = scheme.get(color_str);
	
	//
	if(node->name() == "point") {
		//auto ff = bind(&gfx::draw_point, *g, placeholders::_1, color);
		gui_ui_object uiobj;
		uiobj.draw_calls.emplace_back([this,color](const pnt& offset, const size2& size) {
			cout << "test: !" << endl;
			//g->draw_point(offset, color);
		});
		uiobj.draw_calls.back()(pnt(1,1), size2(10,10));
		//uiobj.draw_calls.emplace_back([&g,this,=](const pnt& offset) { g->draw_point(offset, color); });
	}*/
	/*else if(node->name() == "line") {
	}
	else if(node->name() == "rect") {
	}
	else if(node->name() == "rounded rect") {
	}
	else if(node->name() == "circle") {
	}
	else if(node->name() == "ellipsoid") {
	}*/
}
