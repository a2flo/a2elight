/*
 *  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2013 Florian Ziesche
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

#ifndef __A2E_A2E_SHADER_H__
#define __A2E_A2E_SHADER_H__

#include "global.hpp"

#include "engine.hpp"
#include "rendering/extensions.hpp"
#include "rendering/rtt.hpp"
#include "core/xml.hpp"

#define A2E_SHADER_VERSION 2

/*! @class shader
 *  @brief shader class
 */

class shader;
class A2E_API a2e_shader {
protected:
	enum class CONDITION_TYPE : unsigned int {
		INVALID,
		EQUAL,		//!< enum all conditions must be fulfilled
		NEQUAL,		//!< enum all conditions may not be fulfilled
		OR,			//!< enum one (or more) condtion(s) must be fulfilled
		GEQUAL,		//!< enum greater or equal, only used for graphic card conditions (e.g. condition is "GEQUAL GC_GEFORCE_8" and GC_GEFORCE_9 is specified, the condition is true),
					//!< enum also distinguishes between GC_GEFORCE, GC_RADEON and GC_GENERIC, depending on the specified/used graphic card (so all three GC_* identifiers can be used in one condition)
		LEQUAL,		//!< enum lessor or equal, only used for graphic card conditions (e.g. condition is "LEQUAL GC_RADEON_X" and GC_RADEON_9 is specified, the condition is true)
					//!< enum also distinguishes between GC_GEFORCE, GC_RADEON and GC_GENERIC, depending on the specified/used graphic card (so all three GC_* identifiers can be used in one condition)
		NGEQUAL,	//!< enum not greater or equal
		NLEQUAL		//!< enum not lessor or equal
	};
	
public:
	
	a2e_shader(engine* eng);
	~a2e_shader();
	
	struct a2e_shader_code {
		string header = "";
		string program = "";
		ext::GLSL_VERSION version;
		
		a2e_shader_code() :
#if !defined(A2E_IOS)
		version(ext::GLSL_VERSION::GLSL_150)
#else
		version(ext::GLSL_VERSION::GLSL_ES_100)
#endif
		{}
		a2e_shader_code(a2e_shader_code&& obj) noexcept :
		header(obj.header), program(obj.program), version(obj.version) {}
		a2e_shader_code& operator=(const a2e_shader_code& shd_code) {
			this->header = shd_code.header;
			this->program = shd_code.program;
			this->version = shd_code.version;
			return *this;
		}
	};
	
	struct a2e_shader_object_base {
		string identifier = "";
		vector<string> includes;
		set<string> options;
		set<string> combiners;
		
		// <option*combiner..., code>
		map<string, a2e_shader_code> vertex_shader;
		map<string, a2e_shader_code> geometry_shader;
		map<string, a2e_shader_code> fragment_shader;
		
		void add_option(const string& opt_name) {
			if(options.count(opt_name) != 0) return;
			options.insert(opt_name);
			vertex_shader.insert(make_pair(opt_name, a2e_shader_code()));
			geometry_shader.insert(make_pair(opt_name, a2e_shader_code()));
			fragment_shader.insert(make_pair(opt_name, a2e_shader_code()));
		}
		void remove_option(const string& opt_name) {
			if(options.count(opt_name) == 0) return;
			options.erase(opt_name);
			vertex_shader.erase(opt_name);
			geometry_shader.erase(opt_name);
			fragment_shader.erase(opt_name);
		}
		
		a2e_shader_object_base() {
			add_option("#");
		}
	};
	
	struct a2e_shader_object : public a2e_shader_object_base {
		// <option*combiner..., program>
		map<string, string> vs_program;
		map<string, string> gs_program;
		map<string, string> fs_program;
		bool geometry_shader_available = false;
		
		a2e_shader_object() : a2e_shader_object_base() {}
	};
	
	struct a2e_shader_include_object : public a2e_shader_object_base {
		// everything in a2e_shader_object_base
		a2e_shader_include_object() : a2e_shader_object_base() {}
	};
	
	struct a2e_shader_include {
		string filename;
		a2e_shader_include_object* shader_include_object;
	};
	
	//
	a2e_shader_object* add_a2e_shader(const string& identifier);
	a2e_shader_include_object* create_a2e_shader_include();
	bool load_a2e_shader(const string& identifier, const string& filename, a2e_shader_object* shader_object);
	
	//
	bool compile_a2e_shader(a2e_shader_object* shd);
	bool process_and_compile_a2e_shader(a2e_shader_object* shd);
	
	//
	void get_shader_content(a2e_shader_code& shd, xmlNode* node, const string& option);
	bool check_shader_condition(const CONDITION_TYPE type, const string& value) const;
	CONDITION_TYPE get_condition_type(const string& condition_type) const;

	//
	void load_a2e_shader_includes();
	a2e_shader_object* get_a2e_shader(const string& identifier, const size_t num = 0) {
		if(a2e_shaders.count(identifier) == 0) return nullptr;
		return a2e_shaders[identifier].at(num);
	}
	
	//
	void set_shader_class(shader* shader_obj_) {
		shader_obj = shader_obj_;
	}
	
protected:
	engine* e;
	ext* exts;
	xml* x;
	shader* shader_obj;
	
	vector<a2e_shader_object*> a2e_shader_objects;
	vector<a2e_shader_include_object*> a2e_shader_include_objects;
	map<string, a2e_shader_include*> a2e_shader_includes;
	map<string, vector<a2e_shader_object*>> a2e_shaders;
	
	const map<string, bool> conditions;
	
	void make_glsl_es_compat(a2e_shader_object* shd, const string& option);
	void process_node(const xml::xml_node* cur_node, const xml::xml_node* parent,
					  std::function<void(const xml::xml_node* node)> fnc = [](const xml::xml_node* node floor_unused){});
	
};

#endif
