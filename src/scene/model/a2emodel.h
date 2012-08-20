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

#ifndef __A2E_A2EMODEL_H__
#define __A2E_A2EMODEL_H__

#include "global.h"

#include "core/core.h"
#include "core/vector3.h"
#include "core/bbox.h"
#include "core/file_io.h"
#include "engine.h"
#include "scene/model/a2ematerial.h"
#include "rendering/shader.h"
#include "core/matrix4.h"
#include "scene/light.h"
#include "cl/opencl.h"
#include "rendering/extensions.h"

#define A2E_MAX_MASK_ID 3

/*! @class a2emodel
 *  @brief a2e model base class
 */

class scene;
class A2E_API a2emodel {
public:
	a2emodel(engine* e, shader* s, scene* sce);
	virtual ~a2emodel();
	
	// ret: void, args: draw_mode, sub_object_num
	typedef functor<void, const DRAW_MODE&, const size_t&, const size_t&> draw_callback;
	
	virtual void load_model(const string& filename) = 0;
	virtual const string& get_filename() const;
	
	// draw functions
	virtual void draw(const DRAW_MODE draw_mode) = 0;
	virtual void draw_phys_obj();
	
	// misc model manipulation functions
	virtual void set_position(const float x, const float y, const float z);
	virtual void set_position(const float3& pos);
	virtual void set_rotation(const float x, const float y, const float z);
	virtual void set_rotation(const float3& rot);
	virtual void set_scale(const float x, const float y, const float z);
	virtual void set_scale(const float3& scl);
	virtual void set_hard_scale(const float x, const float y, const float z) = 0;
	virtual void set_hard_scale(const float3& hscl);
	virtual void set_hard_position(const float x, const float y, const float z) = 0;
	virtual void set_hard_position(const float3& hpos);
	virtual void scale_tex_coords(const float su, const float sv) = 0;
	virtual float3& get_position();
	virtual float3& get_scale();
	virtual const float3& get_position() const;
	virtual const float3& get_scale() const;
	virtual void set_rotation_matrix(const matrix4f& mat);
	virtual matrix4f& get_rotation_matrix();
	virtual const matrix4f& get_rotation_matrix() const;
	virtual void update_scale_matrix();
	virtual void build_bounding_box();
	virtual extbbox* get_bounding_box();
	virtual extbbox* get_bounding_box(const size_t& sub_object);
	virtual void set_environment_map(const GLuint env_map);
	virtual GLuint get_environment_map() const;
	
	virtual bool is_collision_model();
	virtual void set_draw_phys_obj(bool state);
	virtual bool get_draw_phys_obj();
	virtual void set_draw_wireframe(bool state);
	virtual bool get_draw_wireframe();
	virtual void set_visible(bool state);
	virtual bool get_visible();
	
	//! note: set/get transparent w/o a specified sub-object applies to the whole model (all sub-objects)
	//! also note that set_transparent overwrites all previously set sub-object transparency flags,
	//! and get_transparent only stores the value of the last set_transparent
	virtual void set_transparent(const bool state);
	virtual bool get_transparent() const;
	virtual void set_transparent(const size_t& sub_object, const bool state);
	virtual bool get_transparent(const size_t& sub_object) const;
	
	virtual void set_material(a2ematerial* material);
	virtual a2ematerial* get_material() const;
	
	// model data functions
	virtual float3** get_vertices() const;
	virtual const float3* get_vertices(unsigned int obj_num) const;
	virtual coord** get_tex_coords() const;
	virtual const coord* get_tex_coords(unsigned int obj_num) const;
	virtual index3** get_indices() const;
	virtual const index3* get_indices(unsigned int obj_num) const;
	virtual unsigned int get_vertex_count() const;
	virtual unsigned int get_vertex_count(unsigned int obj_num) const;
	virtual unsigned int get_index_count() const;
	virtual unsigned int get_index_count(unsigned int obj_num) const;
	virtual string* get_object_names();
	virtual unsigned int get_object_count() const;
	
	// stuff for collision detection
	virtual void set_radius(float radius);
	virtual void set_length(float length);
	virtual float get_radius();
	virtual float get_length();
	virtual void set_phys_scale(float x, float y, float z);
	virtual void set_phys_scale(float3* pscl);
	virtual float3* get_phys_scale();
	
	virtual float3* get_col_vertices();
	virtual index3* get_col_indices();
	virtual unsigned int get_col_vertex_count();
	virtual unsigned int get_col_index_count();
	
	// inferred rendering
	void set_ir_buffers(const rtt::fbo* g_buffer,
						const rtt::fbo* l_buffer,
						const rtt::fbo* g_buffer_alpha,
						const rtt::fbo* l_buffer_alpha);
	
protected:
	// classes
	engine* e;
	texman* t;
	shader* s;
	ext* exts;
	opencl* ocl;
	a2ematerial* material;
	scene* sce;
	string filename = "";
	
	// model 3D/file data, these are actually just pointers to the real 3D data,
	// so no actual memory is allocated for these - they must also be set manually
	// by the derived class (probably in the load_model() function)
	// -> 2D array, containing a pointer for each sub-object
	float3** model_vertices;
	coord** model_tex_coords;
	index3** model_indices;
	unsigned int* model_vertex_count;
	unsigned int* model_index_count;
	
	unsigned int object_count;
	string* object_names;
	
	bool collision_model;
	unsigned int col_vertex_count;
	unsigned int col_index_count;
	float3* col_vertices;
	index3* col_indices;
	
	
	// drawing variables (vbos/buffers), these have to be set by the derived class before calling draw()
	GLuint draw_vertices_vbo;
	GLuint draw_tex_coords_vbo;
	GLuint draw_normals_vbo;
	GLuint draw_binormals_vbo;
	GLuint draw_tangents_vbo;
	GLuint draw_indices_vbo;
	size_t draw_index_count;
	
	//
	enum class VERTEX_ATTRIBUTE : unsigned int {
		NORMAL			= (1 << 0),
		BINORMAL		= (1 << 1),
		TANGENT			= (1 << 2),
		TEXTURE_COORD	= (1 << 3)
	};
	enum_class_bitwise_or(VERTEX_ATTRIBUTE)
	enum_class_bitwise_and(VERTEX_ATTRIBUTE)
	
	// internal draw functions (override these in derived classes if you have to do custom rendering)
	virtual void draw_sub_object(const DRAW_MODE& draw_mode, const size_t& sub_object_num, const size_t& mask_id);
	virtual void ir_mp_setup(gl3shader& shd, const string& option, const set<string>& combiners);
	virtual void pre_draw_setup(const ssize_t sub_object_num = -1); // -1, no sub-object
	virtual void post_draw_setup(const ssize_t sub_object_num = -1);
	virtual void pre_draw_geometry(gl3shader& shd, VERTEX_ATTRIBUTE& attr_array_mask, a2ematerial::TEXTURE_TYPE& texture_mask);
	virtual void post_draw_geometry(gl3shader& shd);
		virtual void pre_draw_material(gl3shader& shd, VERTEX_ATTRIBUTE& attr_array_mask, a2ematerial::TEXTURE_TYPE& texture_mask);
	virtual void post_draw_material(gl3shader& shd);
	//! return an empty string if no custom shader should be used
	virtual const string select_shader(const DRAW_MODE& draw_mode) const;
	
	// orientation
	float3 position;
	float3 scale;
	extbbox bbox;
	extbbox* sub_bboxes;
	void delete_sub_bboxes();
	
	//
	matrix4f rot_mat;
	matrix4f scale_mat;
	matrix4f mvpm; // only used while rendering (global mvpm)
	matrix4f mvpm_backside; // only used while rendering (global mvm)
	matrix4f mvm; // only used while rendering (global mvm)
	
	
	// some flags
	bool draw_wireframe;
	bool is_visible;
	bool is_material;
	bool is_draw_phys_obj;
	bool is_transparent;
	vector<bool> is_sub_object_transparent;
	vector<draw_callback*> transparency_callbacks;
	
	
	// some variables for collision detection
	float radius;
	float length;
	float3 phys_scale;
		
	
	// inferred rendering
	const rtt::fbo* g_buffer;
	const rtt::fbo* l_buffer;
	const rtt::fbo* g_buffer_alpha;
	const rtt::fbo* l_buffer_alpha;
	size_t id;
	
	
	// normal/binormal/tangent generating
	virtual void generate_normals() = 0;
	
	virtual void model_setup();
	
	GLuint env_map = 0;
	
};

#endif
