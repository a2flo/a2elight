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

#ifndef __XML_H__
#define __XML_H__

#include "global.h"
#include "core/core.h"

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

/*! @class xml
 *  @brief xml functions
 */

class engine;
class A2E_API xml {
public:
	xml(engine* e_);
	~xml();

	// new, easy xml document handling
	struct xml_node;
	struct xml_doc {
	private:
		template<typename T> class default_value {
		public: static const T def() { return T(); }
		};
		const string& extract_attr(const string& path) const;
		
	public:
		xml_doc() : nodes(), valid(true) {}
		xml_doc(xml_doc&& doc) {
			nodes.swap(doc.nodes);
			valid = doc.valid;
		}
		xml_doc& operator=(xml_doc&& doc) {
			nodes.swap(doc.nodes);
			valid = doc.valid;
			return *this;
		}
		unordered_multimap<string, xml_node*> nodes;
		bool valid;
		
		xml_node* get_node(const string& path) const;
		
		//! "root.subnode.subnode.attr"
		template<typename T> const T get(const string& path, const T default_val = default_value<T>::def()) const;
	};
	struct xml_node {
		xml_node(const xmlNode* node);
		~xml_node();
		unordered_multimap<string, xml_node*> children;
		unordered_map<string, const string> attributes;
		
		const string& operator[](const string& attr_name) const {
			static const string invalid_attr = "INVALID";
			const auto attr = attributes.find(attr_name);
			if(attr == attributes.end()) return invalid_attr;
			return attr->second;
		}
	};
	xml_doc process_file(const string& filename, const bool validate = true) const;

	// for manual reading
	template <typename T> T get_attribute(const xmlAttribute* attr, const char* attr_name) {
		for(const xmlAttr* cur_attr = (const xmlAttr*)attr; cur_attr; cur_attr = (const xmlAttr*)cur_attr->next) {
			if(strcmp(attr_name, (const char*)cur_attr->name) == 0) {
				return converter<string, T>::convert(string((const char*)cur_attr->children->content));
			}
		}
		a2e_error("element has no attribute named %s!", attr_name);
		return (T)nullptr;
	}
	bool is_attribute(const xmlAttribute* attr, const char* attr_name) {
		for(const xmlAttr* cur_attr = (const xmlAttr*)attr; cur_attr; cur_attr = (const xmlAttr*)cur_attr->next) {
			if(strcmp(attr_name, (const char*)cur_attr->name) == 0) {
				return true;
			}
		}
		return false;
	}

protected:
	engine* e;

};

//
template<> class xml::xml_doc::default_value<string> {
public: static const string def() { return ""; }
};
template<> class xml::xml_doc::default_value<float> {
public: static const float def() { return 0.0f; }
};
template<> class xml::xml_doc::default_value<size_t> {
public: static const size_t def() { return 0; }
};
template<> class xml::xml_doc::default_value<ssize_t> {
public: static const ssize_t def() { return 0; }
};
template<> class xml::xml_doc::default_value<bool> {
public: static const bool def() { return false; }
};
template<> class xml::xml_doc::default_value<float2> {
public: static const float2 def() { return float2(0.0f); }
};
template<> class xml::xml_doc::default_value<float3> {
public: static const float3 def() { return float3(0.0f); }
};
template<> class xml::xml_doc::default_value<float4> {
public: static const float4 def() { return float4(0.0f); }
};
template<> class xml::xml_doc::default_value<size2> {
public: static const size2 def() { return size2(0, 0); }
};
template<> class xml::xml_doc::default_value<size3> {
public: static const size3 def() { return size3(0, 0, 0); }
};
template<> class xml::xml_doc::default_value<size4> {
public: static const size4 def() { return size4(0, 0, 0, 0); }
};
template<> class xml::xml_doc::default_value<ssize2> {
public: static const ssize2 def() { return ssize2(0, 0); }
};
template<> class xml::xml_doc::default_value<ssize3> {
public: static const ssize3 def() { return ssize3(0, 0, 0); }
};
template<> class xml::xml_doc::default_value<ssize4> {
public: static const ssize4 def() { return ssize4(0, 0, 0, 0); }
};

template<> const string xml::xml_doc::get<string>(const string& path, const string default_value) const;
template<> const float xml::xml_doc::get<float>(const string& path, const float default_value) const;
template<> const size_t xml::xml_doc::get<size_t>(const string& path, const size_t default_value) const;
template<> const ssize_t xml::xml_doc::get<ssize_t>(const string& path, const ssize_t default_value) const;
template<> const bool xml::xml_doc::get<bool>(const string& path, const bool default_value) const;
template<> const float2 xml::xml_doc::get<float2>(const string& path, const float2 default_value) const;
template<> const float3 xml::xml_doc::get<float3>(const string& path, const float3 default_value) const;
template<> const float4 xml::xml_doc::get<float4>(const string& path, const float4 default_value) const;
template<> const size2 xml::xml_doc::get<size2>(const string& path, const size2 default_value) const;
template<> const size3 xml::xml_doc::get<size3>(const string& path, const size3 default_value) const;
template<> const size4 xml::xml_doc::get<size4>(const string& path, const size4 default_value) const;
template<> const ssize2 xml::xml_doc::get<ssize2>(const string& path, const ssize2 default_value) const;
template<> const ssize3 xml::xml_doc::get<ssize3>(const string& path, const ssize3 default_value) const;
template<> const ssize4 xml::xml_doc::get<ssize4>(const string& path, const ssize4 default_value) const;

#endif
