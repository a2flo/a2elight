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

#include "xml.h"
#include "engine.h"
#include <libxml/encoding.h>
#include <libxml/catalog.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

/*! create and initialize the xml class
 */
xml::xml(engine* e_) : e(e_) {
	// libxml2 init
	LIBXML_TEST_VERSION
	xmlInitializeCatalog();
	xmlCatalogSetDefaults(XML_CATA_ALLOW_ALL);
	if(xmlCatalogAdd(BAD_CAST "public",
					 BAD_CAST "-//A2E//DTD config 1.0//EN",
					 BAD_CAST ("file://"+e->data_path("dtd/config.dtd")).c_str()) != 0) {
		const auto error_ptr = xmlGetLastError();
		a2e_error("failed to add catalog for config: %s", (error_ptr != nullptr ? error_ptr->message : ""));
	}
	if(xmlCatalogAdd(BAD_CAST "public",
					 BAD_CAST "-//A2E//DTD a2e_shader 2.0//EN",
					 BAD_CAST ("file://"+e->data_path("dtd/a2e_shader.dtd")).c_str()) != 0) {
		const auto error_ptr = xmlGetLastError();
		a2e_error("failed to add catalog for a2e_shader: %s", (error_ptr != nullptr ? error_ptr->message : ""));
	}
}

xml::~xml() {
}

xml::xml_doc xml::process_file(const string& filename, const bool validate) const {
	xml_doc doc;
	
	// read/parse/validate
	xmlParserCtxtPtr ctx = xmlNewParserCtxt();
	if(ctx == nullptr) {
		a2e_error("failed to allocate parser context for \"%s\"!", filename);
		doc.valid = false;
		return doc;
	}
	
	xmlDocPtr xmldoc = xmlCtxtReadFile(ctx, filename.c_str(), nullptr,
									   (validate ? XML_PARSE_DTDLOAD | XML_PARSE_DTDVALID : 0));
	if(xmldoc == nullptr) {
		a2e_error("failed to parse \"%s\"!", filename);
		doc.valid = false;
		return doc;
	}
	else {
		if(ctx->valid == 0) {
			xmlFreeDoc(xmldoc);
			a2e_error("failed to validate \"%s\"!", filename);
			doc.valid = false;
			return doc;
		}
	}
	
	// create internal node structure
	create_node_structure(doc, xmldoc);
	
	// cleanup
	xmlFreeDoc(xmldoc);
	xmlFreeParserCtxt(ctx);
	xmlCleanupParser();
	
	return doc;
}

xml::xml_doc xml::process_data(const string& data, const bool validate) const {
	xml_doc doc;
	
	// read/parse/validate
	xmlParserCtxtPtr ctx = xmlNewParserCtxt();
	if(ctx == nullptr) {
		a2e_error("failed to allocate parser context!");
		doc.valid = false;
		return doc;
	}
	
	xmlDocPtr xmldoc = xmlCtxtReadDoc(ctx, (const xmlChar*)data.c_str(), e->data_path("dtd/shader.xml").c_str(), nullptr,
									  (validate ? XML_PARSE_DTDLOAD | XML_PARSE_DTDVALID : 0));
	if(xmldoc == nullptr) {
		a2e_error("failed to parse data!");
		doc.valid = false;
		return doc;
	}
	else {
		if(ctx->valid == 0) {
			xmlFreeDoc(xmldoc);
			a2e_error("failed to validate data!");
			doc.valid = false;
			return doc;
		}
	}
	
	// create internal node structure
	create_node_structure(doc, xmldoc);
	
	// cleanup
	xmlFreeDoc(xmldoc);
	xmlFreeParserCtxt(ctx);
	xmlCleanupParser();
	
	return doc;
}

void xml::create_node_structure(xml::xml_doc& doc, xmlDocPtr xmldoc) const {
	deque<pair<xmlNode*, unordered_multimap<string, xml_node*>*>> node_stack;
	node_stack.push_back(make_pair(xmldoc->children, &doc.nodes));
	for(;;) {
		if(node_stack.empty()) break;
		
		xmlNode* cur_node = node_stack.front().first;
		unordered_multimap<string, xml_node*>* nodes = node_stack.front().second;
		node_stack.pop_front();
		
		for(; cur_node; cur_node = cur_node->next) {
			if(cur_node->type == XML_ELEMENT_NODE) {
				xml_node* node = new xml_node(cur_node);
				nodes->insert(make_pair(string((const char*)cur_node->name), node));
				
				if(cur_node->children != nullptr) {
					node_stack.push_back(make_pair(cur_node->children, &node->children));
				}
			}
		}
	}
}

xml::xml_node* xml::xml_doc::get_node(const string& path) const {
	// find the node
	vector<string> levels = core::tokenize(path, '.');
	const unordered_multimap<string, xml_node*>* cur_level = &nodes;
	xml_node* cur_node = nullptr;
	for(const string& level : levels) {
		const auto& next_node = cur_level->find(level);
		if(next_node == cur_level->end()) return nullptr;
		cur_node = next_node->second;
		cur_level = &cur_node->children;
	}
	return cur_node;
}

const string& xml::xml_doc::extract_attr(const string& path) const {
	static const string invalid_attr = "INVALID";
	const size_t lp = path.rfind(".");
	if(lp == string::npos) return invalid_attr;
	const string node_path = path.substr(0, lp);
	const string attr_name = path.substr(lp+1, path.length()-lp-1);
	
	xml_node* node = get_node(node_path);
	if(node == nullptr) return invalid_attr;
	return (attr_name != "content" ? (*node)[attr_name] : node->content());
}

template<> const string xml::xml_doc::get<string>(const string& path, const string default_value) const {
	const string& attr = extract_attr(path);
	return (attr != "INVALID" ? attr : default_value);
}
template<> const float xml::xml_doc::get<float>(const string& path, const float default_value) const {
	const string& attr = extract_attr(path);
	return (attr != "INVALID" ? strtof(attr.c_str(), nullptr) : default_value);
}
template<> const size_t xml::xml_doc::get<size_t>(const string& path, const size_t default_value) const {
	const string& attr = extract_attr(path);
	return (attr != "INVALID" ? strtoull(attr.c_str(), nullptr, 10) : default_value);
}
template<> const ssize_t xml::xml_doc::get<ssize_t>(const string& path, const ssize_t default_value) const {
	const string& attr = extract_attr(path);
	return (attr != "INVALID" ? strtoll(attr.c_str(), nullptr, 10) : default_value);
}
template<> const bool xml::xml_doc::get<bool>(const string& path, const bool default_value) const {
	const string& attr = extract_attr(path);
	return (attr != "INVALID" ?
			(attr == "yes" || attr == "YES" ||
			 attr == "true" || attr == "TRUE" ||
			 attr == "on" || attr == "ON" || attr == "1" ? true : false) : default_value);
}
template<> const float2 xml::xml_doc::get<float2>(const string& path, const float2 default_value) const {
	const string& attr = extract_attr(path);
	vector<string> tokens= core::tokenize(attr, ',');
	return (attr != "INVALID" && tokens.size() >= 2 ?
			float2(strtof(tokens[0].c_str(), nullptr), strtof(tokens[1].c_str(), nullptr))
			: default_value);
}
template<> const float3 xml::xml_doc::get<float3>(const string& path, const float3 default_value) const {
	const string& attr = extract_attr(path);
	vector<string> tokens= core::tokenize(attr, ',');
	return (attr != "INVALID" && tokens.size() >= 3 ?
			float3(strtof(tokens[0].c_str(), nullptr), strtof(tokens[1].c_str(), nullptr), strtof(tokens[2].c_str(), nullptr))
			: default_value);
}
template<> const float4 xml::xml_doc::get<float4>(const string& path, const float4 default_value) const {
	const string& attr = extract_attr(path);
	vector<string> tokens= core::tokenize(attr, ',');
	return (attr != "INVALID" && tokens.size() >= 4 ?
			float4(strtof(tokens[0].c_str(), nullptr), strtof(tokens[1].c_str(), nullptr), strtof(tokens[2].c_str(), nullptr), strtof(tokens[3].c_str(), nullptr))
			: default_value);
}
template<> const size2 xml::xml_doc::get<size2>(const string& path, const size2 default_value) const {
	const string& attr = extract_attr(path);
	vector<string> tokens= core::tokenize(attr, ',');
	return (attr != "INVALID" && tokens.size() >= 2 ?
			size2(strtoull(tokens[0].c_str(), nullptr, 10),
				  strtoull(tokens[1].c_str(), nullptr, 10))
			: default_value);
}
template<> const size3 xml::xml_doc::get<size3>(const string& path, const size3 default_value) const {
	const string& attr = extract_attr(path);
	vector<string> tokens= core::tokenize(attr, ',');
	return (attr != "INVALID" && tokens.size() >= 3 ?
			size3(strtoull(tokens[0].c_str(), nullptr, 10),
				  strtoull(tokens[1].c_str(), nullptr, 10),
				  strtoull(tokens[2].c_str(), nullptr, 10))
			: default_value);
}
template<> const size4 xml::xml_doc::get<size4>(const string& path, const size4 default_value) const {
	const string& attr = extract_attr(path);
	vector<string> tokens= core::tokenize(attr, ',');
	return (attr != "INVALID" && tokens.size() >= 4 ?
			size4(strtoull(tokens[0].c_str(), nullptr, 10),
				  strtoull(tokens[1].c_str(), nullptr, 10),
				  strtoull(tokens[2].c_str(), nullptr, 10),
				  strtoull(tokens[3].c_str(), nullptr, 10))
			: default_value);
}
template<> const ssize2 xml::xml_doc::get<ssize2>(const string& path, const ssize2 default_value) const {
	const string& attr = extract_attr(path);
	vector<string> tokens= core::tokenize(attr, ',');
	return (attr != "INVALID" && tokens.size() >= 2 ?
			ssize2(strtoll(tokens[0].c_str(), nullptr, 10),
				   strtoll(tokens[1].c_str(), nullptr, 10))
			: default_value);
}
template<> const ssize3 xml::xml_doc::get<ssize3>(const string& path, const ssize3 default_value) const {
	const string& attr = extract_attr(path);
	vector<string> tokens= core::tokenize(attr, ',');
	return (attr != "INVALID" && tokens.size() >= 3 ?
			ssize3(strtoll(tokens[0].c_str(), nullptr, 10),
				   strtoll(tokens[1].c_str(), nullptr, 10),
				   strtoll(tokens[2].c_str(), nullptr, 10))
			: default_value);
}
template<> const ssize4 xml::xml_doc::get<ssize4>(const string& path, const ssize4 default_value) const {
	const string& attr = extract_attr(path);
	vector<string> tokens= core::tokenize(attr, ',');
	return (attr != "INVALID" && tokens.size() >= 4 ?
			ssize4(strtoll(tokens[0].c_str(), nullptr, 10),
				   strtoll(tokens[1].c_str(), nullptr, 10),
				   strtoll(tokens[2].c_str(), nullptr, 10),
				   strtoll(tokens[3].c_str(), nullptr, 10))
			: default_value);
}

xml::xml_node::xml_node(const xmlNode* node) :
node_name((const char*)node->name),
node_content(xmlNodeGetContent((xmlNodePtr)node) != nullptr ? (const char*)xmlNodeGetContent((xmlNodePtr)node) : "") {
	for(xmlAttr* cur_attr = node->properties; cur_attr; cur_attr = cur_attr->next) {
		attributes.insert(make_pair(string((const char*)cur_attr->name),
									string((cur_attr->children != nullptr ? (const char*)cur_attr->children->content : "INVALID"))
									));
	}
}
xml::xml_node::~xml_node() {
	// cascading delete
	for(const auto& child : children) {
		delete child.second;
	}
	attributes.clear();
}
