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

#include "shader_gles2.h"

#if A2E_DEBUG // type checking in debug mode

#define __A2E_DECLARE_GL_TYPE_TO_STRING(type) case type: return #type;

const char* gl_type_to_string(const size_t& type); // prototype ...
const char* gl_type_to_string(const size_t& type) {
	switch(type) {
			A2E_GL_SHADER_TYPES(__A2E_DECLARE_GL_TYPE_TO_STRING);
		default: break;
	}
	stringstream buffer;
	buffer << hex << uppercase << (size_t)type;
	return buffer.str().c_str();
}

#define A2E_CHECK_UNIFORM_EXISTENCE(name) \
if(shd_obj.programs.size() <= cur_program) { \
a2e_error("invalid program #%u for shader \"%s\"!", cur_program, shd_obj.name.c_str()); \
return; \
} \
if(shd_obj.programs[cur_program]->uniforms.count(name) == 0) { \
a2e_error("unknown uniform name \"%s\" for shader \"%s\"!", name, shd_obj.name.c_str()); \
return; \
}

#define A2E_CHECK_ATTRIBUTE_EXISTENCE(name) \
if(shd_obj.programs.size() <= cur_program) { \
a2e_error("invalid program #%u for shader \"%s\"!", cur_program, shd_obj.name.c_str()); \
return; \
} \
if(shd_obj.programs[cur_program]->attributes.count(name) == 0) { \
a2e_error("unknown attribute name \"%s\" for shader \"%s\"!", name, shd_obj.name.c_str()); \
return; \
}

#define A2E_CHECK_BLOCK_EXISTENCE(name) \
if(shd_obj.programs.size() <= cur_program) { \
a2e_error("invalid program #%u for shader \"%s\"!", cur_program, shd_obj.name.c_str()); \
return; \
} \
if(shd_obj.programs[cur_program]->blocks.count(name) == 0) { \
a2e_error("unknown uniform block name \"%s\" for shader \"%s\"!", name, shd_obj.name.c_str()); \
return; \
}

#define A2E_CHECK_UNIFORM_TYPE(name, expected_type) \
size_t uniform_type = shd_obj.programs[cur_program]->uniforms.find(name)->second.type; \
if(uniform_type != (size_t)expected_type) { \
a2e_error("unexpected type %s for uniform \"%s\" - expected %s (in shader \"%s\")!", gl_type_to_string(uniform_type), name, gl_type_to_string(expected_type), shd_obj.name.c_str()); \
}

#define A2E_CHECK_ATTRIBUTE_TYPE(name, expected_type) \
size_t attribute_type = shd_obj.programs[cur_program]->attributes.find(name)->second.type; \
if(attribute_type != (size_t)expected_type) { \
a2e_error("unexpected type %s for attribute \"%s\" - expected %s (in shader \"%s\")!", gl_type_to_string(attribute_type), name, gl_type_to_string(expected_type), shd_obj.name.c_str()); \
}

#else // don't check the type in release mode
#define A2E_CHECK_UNIFORM_EXISTENCE(name)
#define A2E_CHECK_ATTRIBUTE_EXISTENCE(name)
#define A2E_CHECK_BLOCK_EXISTENCE(name)
#define A2E_CHECK_UNIFORM_TYPE(name, expected_type)
#define A2E_CHECK_ATTRIBUTE_TYPE(name, expected_type)
#endif

///////////////////////////////////////////////////////////////////////////////////////
// shader_gles2 functions
shader_gles2::shader_gles2(const shader_object& shd_obj_) : basic_shader(shd_obj_) {
	use(0);
#if A2E_DEBUG
	if(shd_obj.programs.size() == 0) {
		a2e_error("shader \"%s\" has no programs!", shd_obj.name.c_str());
	}
#endif
}

void shader_gles2::disable() {
	// disable all set vertex attribute arrays
	for(const auto& vattr_iter : active_vertex_attribs) {
		glDisableVertexAttribArray((GLuint)vattr_iter);
	}
	if(!active_vertex_attribs.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	active_vertex_attribs.clear();
	
	// disable program
	glUseProgram(0);
}

void shader_gles2::use() {
	shader_gles2::use(0);
}

void shader_gles2::use(const size_t& program) {
	shader_base::use(program);
	glUseProgram(shd_obj.programs[cur_program]->program);
#if A2E_DEBUG
	if(shd_obj.programs.size() == 0) {
		a2e_error("no program #%u exists in shader \"%s\"!", program, shd_obj.name.c_str());
	}
#endif
}

void shader_gles2::use(const string& option) {
	cur_option = option;
#if A2E_DEBUG
	if(shd_obj.options.count(option) == 0) {
		a2e_error("no option \"%s\" exists in shader \"%s\"!", option, shd_obj.name);
		return;
	}
#endif
	const shader_object::internal_shader_object* int_shd_obj = shd_obj.options.find(option)->second;
	const auto piter = find(shd_obj.programs.begin(), shd_obj.programs.end(), int_shd_obj);
	cur_program = piter - shd_obj.programs.begin();
	use(cur_program);
}

size_t shader_gles2::get_cur_program() const {
	return cur_program;
}

const string& shader_gles2::get_cur_option() const {
	return cur_option;
}

///////////////////////////////////////////////////////////////////////////////////////
// -> uniform

// 1{i,f,b,fv,iv,bv}
void shader_gles2::uniform(const char* name, const float& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT);
	glUniform1f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1);
}

void shader_gles2::uniform(const char* name, const int& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT);
	glUniform1i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1);
}

void shader_gles2::uniform(const char* name, const bool& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL);
	glUniform1i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1);
}

void shader_gles2::uniform(const char* name, const float* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT);
	glUniform1fv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, arg1);
}

void shader_gles2::uniform(const char* name, const int* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT);
	glUniform1iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, arg1);
}

void shader_gles2::uniform(const char* name, const bool* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL);
	GLint* int_array = new int[count];
	for(size_t i = 0; i < count; i++) { int_array[i] = arg1[i]; }
	glUniform1iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, int_array);
	delete [] int_array;
}

// 2{i,f,b,fv,iv,bv}
void shader_gles2::uniform(const char* name, const float& arg1, const float& arg2) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC2);
	glUniform2f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2);
}

void shader_gles2::uniform(const char* name, const int& arg1, const int& arg2) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC2);
	glUniform2i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2);
}

void shader_gles2::uniform(const char* name, const bool& arg1, const bool& arg2) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC2);
	glUniform2i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2);
}

void shader_gles2::uniform(const char* name, const float2& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC2);
	glUniform2f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y);
}

void shader_gles2::uniform(const char* name, const int2& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC2);
	glUniform2i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y);
}

void shader_gles2::uniform(const char* name, const bool2& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC2);
	glUniform2i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y);
}

void shader_gles2::uniform(const char* name, const float2* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC2);
	glUniform2fv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLfloat*)arg1);
}

void shader_gles2::uniform(const char* name, const int2* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC2);
	glUniform2iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLint*)arg1);
}

void shader_gles2::uniform(const char* name, const bool2* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC2);
	GLint* int_array = new int[count*2];
	for(size_t i = 0; i < (count*2); i++) {
		int_array[i] = arg1[i].x;
		i++;
		int_array[i] = arg1[i].y;
	}
	glUniform1iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, int_array);
	delete [] int_array;
}


// 3{i,f,b,fv,iv,bv}
void shader_gles2::uniform(const char* name, const float& arg1, const float& arg2, const float& arg3) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC3);
	glUniform3f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3);
}

void shader_gles2::uniform(const char* name, const int& arg1, const int& arg2, const int& arg3) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC3);
	glUniform3i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3);
}

void shader_gles2::uniform(const char* name, const bool& arg1, const bool& arg2, const bool& arg3) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC3);
	glUniform3i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3);
}

void shader_gles2::uniform(const char* name, const float3& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC3);
	glUniform3f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z);
}

void shader_gles2::uniform(const char* name, const int3& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC3);
	glUniform3i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z);
}

void shader_gles2::uniform(const char* name, const bool3& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC3);
	glUniform3i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z);
}

void shader_gles2::uniform(const char* name, const float3* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC3);
	glUniform3fv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLfloat*)arg1);
}

void shader_gles2::uniform(const char* name, const int3* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC3);
	glUniform3iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLint*)arg1);
}

void shader_gles2::uniform(const char* name, const bool3* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC3);
	GLint* int_array = new int[count*3];
	for(size_t i = 0; i < (count*3); i++) {
		int_array[i] = arg1[i].x;
		i++;
		int_array[i] = arg1[i].y;
	}
	glUniform3iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, int_array);
	delete [] int_array;
}


// 4{i,f,b,fv,iv,bv}
void shader_gles2::uniform(const char* name, const float& arg1, const float& arg2, const float& arg3, const float& arg4) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC4);
	glUniform4f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gles2::uniform(const char* name, const int& arg1, const int& arg2, const int& arg3, const int& arg4) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC4);
	glUniform4i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gles2::uniform(const char* name, const bool& arg1, const bool& arg2, const bool& arg3, const bool& arg4) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC4);
	glUniform4i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gles2::uniform(const char* name, const float4& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC4);
	glUniform4f(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z, arg1.w);
}

void shader_gles2::uniform(const char* name, const int4& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC4);
	glUniform4i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z, arg1.w);
}

void shader_gles2::uniform(const char* name, const bool4& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC4);
	glUniform4i(A2E_SHADER_GET_UNIFORM_POSITION(name), arg1.x, arg1.y, arg1.z, arg1.w);
}

void shader_gles2::uniform(const char* name, const float4* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_VEC4);
	glUniform4fv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLfloat*)arg1);
}

void shader_gles2::uniform(const char* name, const int4* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_INT_VEC4);
	glUniform4iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, (GLint*)arg1);
}

void shader_gles2::uniform(const char* name, const bool4* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_BOOL_VEC4);
	GLint* int_array = new int[count*4];
	for(size_t i = 0; i < (count*4); i++) {
		int_array[i] = arg1[i].x;
		i++;
		int_array[i] = arg1[i].y;
	}
	glUniform4iv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, int_array);
	delete [] int_array;
}

// mat{--3,4}
void shader_gles2::uniform(const char* name, const matrix4f& arg1) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_MAT4);
	glUniformMatrix4fv(A2E_SHADER_GET_UNIFORM_POSITION(name), 1, false, (GLfloat*)arg1.data);
}

void shader_gles2::uniform(const char* name, const matrix4f* arg1, const size_t& count) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
	A2E_CHECK_UNIFORM_TYPE(name, GL_FLOAT_MAT4);
	glUniformMatrix4fv(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLsizei)count, false, (GLfloat*)arg1);
}

///////////////////////////////////////////////////////////////////////////////////////
// -> texture

void shader_gles2::set_texture(const char* name, const GLuint& tex, const GLenum& texture_type) const {
	A2E_CHECK_UNIFORM_EXISTENCE(name);
#if A2E_DEBUG
	// check texture number (0 == uninitialized)
	if(tex == 0) {
		a2e_error("invalid texture number %u for texture uniform \"%s\" (in shader \"%s\")!", tex, name, shd_obj.name.c_str());
	}
	// check type
	const size_t uniform_type = shd_obj.programs[cur_program]->uniforms.find(name)->second.type;
	if(!is_gl_sampler_type((const GLenum)uniform_type)) {
		a2e_error("unexpected type %s for texture uniform \"%s\" - expected a sampler type (in shader \"%s\")!", gl_type_to_string(uniform_type), name, shd_obj.name.c_str());
	}
	// check sampler mapping existence
	if(shd_obj.programs[cur_program]->samplers.count(name) == 0) {
		a2e_error("no sampler mapping for texture uniform \"%s\" exists (in shader \"%s\")!", name, shd_obj.name.c_str());
	}
#endif
	
	const size_t tex_num = shd_obj.programs[cur_program]->samplers.find(name)->second;
#if A2E_DEBUG
	if(tex_num >= 32) {
		a2e_error("invalid texture number #%u for texture uniform \"%s\" - only 32 textures are allowed (in shader \"%s\")!", tex_num, name, shd_obj.name.c_str());
	}
#endif
	
	// set uniform, activate and bind texture
	glUniform1i(A2E_SHADER_GET_UNIFORM_POSITION(name), (GLint)tex_num);
	glActiveTexture(GL_TEXTURE0+(GLint)tex_num);
	glBindTexture(texture_type, tex);
}

void shader_gles2::texture(const char* name, const GLuint& tex, const GLenum texture_type) const {
	set_texture(name, tex, texture_type);
}

void shader_gles2::texture(const char* name, const a2e_texture& tex) const {
	set_texture(name, tex->tex_num, tex->texture_type);
}

///////////////////////////////////////////////////////////////////////////////////////
// -> attribute

// NOTE: in opengl 2.x attributes types must be of type float! => overwrite these functions in opengl 3.x+

// 1{f,fv}
void shader_gles2::attribute(const char* name, const float& arg1) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT);
	glVertexAttrib1f(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1);
}

void shader_gles2::attribute(const char* name, const float* arg1, const size_t& count) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT);
	glVertexAttrib1fv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1);
}

// 2{f,fv}
void shader_gles2::attribute(const char* name, const float& arg1, const float& arg2) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC2);
	glVertexAttrib2f(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2);
}

void shader_gles2::attribute(const char* name, const float2* arg1, const size_t& count) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC2);
	glVertexAttrib2fv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLfloat*)arg1);
}

// 3{f,fv}
void shader_gles2::attribute(const char* name, const float& arg1, const float& arg2, const float& arg3) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC3);
	glVertexAttrib3f(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2, arg3);
}

void shader_gles2::attribute(const char* name, const float3* arg1, const size_t& count) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC3);
	glVertexAttrib3fv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLfloat*)arg1);
}

// 4{f,fv}
void shader_gles2::attribute(const char* name, const float& arg1, const float& arg2, const float& arg3, const float& arg4) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4f(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), arg1, arg2, arg3, arg4);
}

void shader_gles2::attribute(const char* name, const float4* arg1, const size_t& count) const {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	A2E_CHECK_ATTRIBUTE_TYPE(name, GL_FLOAT_VEC4);
	glVertexAttrib4fv(A2E_SHADER_GET_ATTRIBUTE_POSITION(name), (GLfloat*)arg1);
}

///////////////////////////////////////////////////////////////////////////////////////
// -> attribute array

void shader_gles2::attribute_array(const char* name, const GLuint& buffer, const GLint& size, const GLenum type, const GLboolean normalized, const GLsizei stride) {
	A2E_CHECK_ATTRIBUTE_EXISTENCE(name);
	
	// SHADER TODO: type/size via shader obj?
	
	const size_t location = A2E_SHADER_GET_ATTRIBUTE_POSITION(name);
	active_vertex_attribs.insert(location);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableVertexAttribArray((GLuint)location);
	if(size <= 4) {
		glVertexAttribPointer((GLuint)location, size, type, normalized, stride, nullptr);
	}
	else if(size == 9) {
		glVertexAttribPointer((GLuint)location, 3, type, normalized, 36, nullptr);
		glEnableVertexAttribArray((GLuint)location+1);
		glVertexAttribPointer((GLuint)location+1, 3, type, normalized, 36, (void*)12);
		glEnableVertexAttribArray((GLuint)location+2);
		glVertexAttribPointer((GLuint)location+2, 3, type, normalized, 36, (void*)24);
	}
	else if(size == 16) {
		glVertexAttribPointer((GLuint)location, 4, type, normalized, 64, nullptr);
		glEnableVertexAttribArray((GLuint)location+1);
		glVertexAttribPointer((GLuint)location+1, 4, type, normalized, 64, (void*)16);
		glEnableVertexAttribArray((GLuint)location+2);
		glVertexAttribPointer((GLuint)location+2, 4, type, normalized, 64, (void*)32);
		glEnableVertexAttribArray((GLuint)location+3);
		glVertexAttribPointer((GLuint)location+3, 4, type, normalized, 64, (void*)48);
	}
	// SHADER TODO: support for other matrix types
}

///////////////////////////////////////////////////////////////////////////////////////
// -> uniform buffer

void shader_gles2::block(const char* name, const GLuint& ubo) const {
	a2e_error("UBOs are not supported in OpenGL ES 2.0!");
}
