/*
 *  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2014 Florian Ziesche
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

#ifndef __A2E_A2ESTATIC_HPP__
#define __A2E_A2ESTATIC_HPP__

#include "global.hpp"

#include "core/core.hpp"
#include "math/vector_lib.hpp"
#include "math/bbox.hpp"
#include "core/file_io.hpp"
#include "engine.hpp"
#include "scene/model/a2ematerial.hpp"
#include "rendering/shader.hpp"
#include "math/matrix4.hpp"
#include "scene/light.hpp"
#include "scene/model/a2emodel.hpp"

//! class for loading and displaying an a2e static model
class a2estatic : public a2emodel {
public:
	a2estatic(shader* s, scene* sce);
	virtual ~a2estatic();

	virtual void draw(const DRAW_MODE draw_mode);
	virtual void load_model(const string& filename);
	void load_from_memory(unsigned int object_count, unsigned int vertex_count,
						  float3* vertices, coord* tex_coords,
						  unsigned int* index_count, index3** indices);
	
	virtual void set_hard_scale(const float x, const float y, const float z);
	virtual void set_hard_position(const float x, const float y, const float z);
	virtual void scale_tex_coords(const float su, const float sv);

	float3* get_col_vertices();
	index3* get_col_indices();
	unsigned int get_col_vertex_count();
	unsigned int get_col_index_count();

	//
	GLuint get_vbo_vertices() const { return vbo_vertices_id; }
	GLuint get_vbo_tex_coords() const { return vbo_tex_coords_id; }
	GLuint get_vbo_indices(const size_t& sub_object) const { return vbo_indices_ids[sub_object]; }
	GLuint get_vbo_normals() const { return vbo_normals_id; }
	GLuint get_vbo_binormals() const { return vbo_binormals_id; }
	GLuint get_vbo_tangents() const { return vbo_tangents_id; }

protected:
	float3* vertices;
	coord* tex_coords;
	index3** indices;
	index3** tex_indices;
	unsigned int vertex_count;
	unsigned int tex_coord_count;
	unsigned int* index_count;
	unsigned int* min_index;
	unsigned int* max_index;
	GLuint vbo_vertices_id;
	GLuint vbo_tex_coords_id;
	GLuint* vbo_indices_ids;
	GLuint vbo_normals_id;
	GLuint vbo_binormals_id;
	GLuint vbo_tangents_id;

	float3* normals;
	float3* binormals;
	float3* tangents;

	// normal stuff
	void reorganize_model_data();

	// used for parallax mapping
	void generate_normals();
	
	virtual void pre_draw_setup(const ssize_t sub_object_num = -1);
	virtual void post_draw_setup(const ssize_t sub_object_num = -1);
	
};

#endif
