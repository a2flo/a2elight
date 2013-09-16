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

#ifndef __A2E_SHADER_OBJECT_H__
#define __A2E_SHADER_OBJECT_H__

#include "global.h"

struct shader_object {
	struct internal_shader_object {
		GLuint program;
		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint geometry_shader;
		GLuint tess_control_shader;
		GLuint tess_evaluation_shader;
		
		struct shader_variable {
			size_t location;
			size_t size;
			size_t type;
			shader_variable(size_t location_, size_t size_, GLenum type_) : location(location_), size(size_), type(type_) {}
		};
		map<string, shader_variable> uniforms;
		map<string, shader_variable> attributes;
		map<string, size_t> samplers;
		map<string, shader_variable> blocks;
		
		internal_shader_object() : program(0), vertex_shader(0), fragment_shader(0), geometry_shader(0), tess_control_shader(0), tess_evaluation_shader(0), uniforms(), attributes(), samplers(), blocks() {}
		~internal_shader_object() {
			// TODO: delete shaders?
			uniforms.clear();
			attributes.clear();
			samplers.clear();
			blocks.clear();
		}
	};
	string name;
	vector<internal_shader_object*> programs;
	map<string, internal_shader_object*> options;
	ext::GLSL_VERSION glsl_version;
	bool a2e_shader;
	
	shader_object(const string& shd_name) : name(shd_name), programs(), options(),
#if !defined(A2E_IOS)
	glsl_version(ext::GLSL_VERSION::GLSL_150),
#else
	glsl_version(ext::GLSL_VERSION::GLSL_ES_100),
#endif
	a2e_shader(false) {}
	~shader_object() {
		for(const auto& prog : programs) {
			delete prog;
		}
		programs.clear();
	}
};

#endif
