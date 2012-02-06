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

#ifndef __A2E_SHADER_H__
#define __A2E_SHADER_H__

#include "global.h"

#include "engine.h"
#include "rendering/extensions.h"
#include "rendering/rtt.h"
#include "core/xml.h"

#define A2E_SHADER_VERSION 1

/*! @class shader
 *  @brief shader class
 */

class shader;
class A2E_API a2e_shader {
protected:

	enum A2E_SHADER_CONDITION_TYPE {
		INVALID,
		EQUAL,		//!< enum all conditions must be fulfilled
		NEQUAL,		//!< enum all conditions may not be fulfilled
		OR,			//!< enum one (ore more) condtion(s) must be fulfilled
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
	
	enum SHADER_PREPROCESSING {
		SP_NONE,
		SP_LIGHTING,
		SP_GUI
	};
	
	struct a2e_shader_code {
		string preprocessor;
		string variables;
		string program;
		ext::GLSL_VERSION version;
		SHADER_PREPROCESSING preprocessing;
		
		a2e_shader_code() : preprocessor(""), variables(""), program(""), version(ext::GLSL_150), preprocessing(SP_NONE) {}
		a2e_shader_code& operator=(const a2e_shader_code& shd_code) {
			this->preprocessor = shd_code.preprocessor;
			this->variables = shd_code.variables;
			this->program = shd_code.program;
			this->version = shd_code.version;
			this->preprocessing = shd_code.preprocessing;
			return *this;
		}
	};
	
	struct a2e_shader_object_base {
		// <option, code>
		map<string, a2e_shader_code*> vertex_shader;
		map<string, a2e_shader_code*> geometry_shader;
		map<string, a2e_shader_code*> fragment_shader;
		string identifier;
		set<string> options;
		
		void add_option(const string& opt_name) {
			if(options.count(opt_name) != 0) return;
			options.insert(opt_name);
			vertex_shader[opt_name] = new a2e_shader_code();
			geometry_shader[opt_name] = new a2e_shader_code();
			fragment_shader[opt_name] = new a2e_shader_code();
		}
		void remove_option(const string& opt_name) {
			if(options.count(opt_name) == 0) return;
			options.erase(opt_name);
			delete vertex_shader[opt_name];
			delete geometry_shader[opt_name];
			delete fragment_shader[opt_name];
			vertex_shader.erase(opt_name);
			geometry_shader.erase(opt_name);
			fragment_shader.erase(opt_name);
		}
		
		a2e_shader_object_base() : vertex_shader(), geometry_shader(), fragment_shader(),
		identifier(""), options() {
			add_option("#");
		}
		virtual ~a2e_shader_object_base() {
			for(auto& shd : vertex_shader) delete shd.second;
			for(auto& shd : geometry_shader) delete shd.second;
			for(auto& shd : fragment_shader) delete shd.second;
		}
	};
	
	struct a2e_shader_object : public a2e_shader_object_base {
		vector<string> includes;
		// <option, program>
		map<string, string> vs_program;
		map<string, string> gs_program;
		map<string, string> fs_program;
		bool geometry_shader_available; // TODO: needed?
		
		a2e_shader_object() : a2e_shader_object_base(), includes(), vs_program(), gs_program(), fs_program(), geometry_shader_available(false) {}
	};
	
	struct a2e_shader_include_object : public a2e_shader_object_base {
		// everything in a2e_shader_object_base
		a2e_shader_include_object() : a2e_shader_object_base() {}
	};
	
	struct a2e_shader_include{
		string filename;
		a2e_shader_include_object* shader_include_object;
	};
	
	//
	a2e_shader_object* add_a2e_shader(const string& identifier);
	a2e_shader_include_object* create_a2e_shader_include();
	bool load_a2e_shader(const string& identifier, const string& filename, a2e_shader_object* shader_object);
	
	//
	bool compile_a2e_shader(a2e_shader_object* shd);
	bool preprocess_and_compile_a2e_shader(a2e_shader_object* shd);
	
	//
	void get_shader_data(a2e_shader_code* shd, const char* shader_type);
	void get_shader_content(a2e_shader_code* shd, xmlNode* node, const string& option);
	bool check_shader_condition(const A2E_SHADER_CONDITION_TYPE type, const string& value);
	A2E_SHADER_CONDITION_TYPE get_condition_type(const string& condition_type);

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
	file_io* f;
	ext* exts;
	xml* x;
	shader* shader_obj;
	
	stringstream buffer;
	stringstream buffer2;
	
	vector<a2e_shader_object*> a2e_shader_objects;
	vector<a2e_shader_include_object*> a2e_shader_include_objects;
	map<string, a2e_shader_include*> a2e_shader_includes;
	map<string, vector<a2e_shader_object*> > a2e_shaders;
	
	map<string, bool> conditions;
	
};

#endif
