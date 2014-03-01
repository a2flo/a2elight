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

#include "a2estatic.hpp"

/*! a2estatic constructor
 */
a2estatic::a2estatic(shader* s, scene* sce) : a2emodel(s, sce) {
	vertices = nullptr;
	tex_coords = nullptr;
	indices = nullptr;
	vertex_count = 0;
	index_count = nullptr;
	min_index = nullptr;
	max_index = nullptr;
	vbo_indices_ids = nullptr;
	
	normals = nullptr;
	binormals = nullptr;
	tangents = nullptr;
}

/*! a2estatic destructor
 */
a2estatic::~a2estatic() {
	if(vertices != nullptr) { delete [] vertices; }
	if(tex_coords != nullptr) { delete [] tex_coords; }
	if(indices != nullptr) { // no additional delete for tex_indices needed, b/c it points to the same data as indices
	    for(unsigned int i = 0; i < object_count; i++) {
	        delete [] indices[i];
	    }
	    delete [] indices;
    }
	if(index_count != nullptr) { delete [] index_count; }
	if(min_index != nullptr) { delete [] min_index; }
	if(max_index != nullptr) { delete [] max_index; }

	if(normals != nullptr) { delete [] normals; }
	if(binormals != nullptr) { delete [] binormals; }
	if(tangents != nullptr) { delete [] tangents; }
	
	if(model_vertices != nullptr) { delete [] model_vertices; }
	if(model_vertex_count != nullptr) { delete [] model_vertex_count; }

	// delete vbos
	if(glIsBuffer(vbo_vertices_id)) { glDeleteBuffers(1, &vbo_vertices_id); }
	if(glIsBuffer(vbo_tex_coords_id)) { glDeleteBuffers(1, &vbo_tex_coords_id); }
	if(vbo_indices_ids != nullptr) {
		if(glIsBuffer(vbo_indices_ids[0])) { glDeleteBuffers(object_count, vbo_indices_ids); }
		delete [] vbo_indices_ids;
	}
	if(glIsBuffer(vbo_normals_id)) { glDeleteBuffers(1, &vbo_normals_id); }
	if(glIsBuffer(vbo_binormals_id)) { glDeleteBuffers(1, &vbo_binormals_id); }
	if(glIsBuffer(vbo_tangents_id)) { glDeleteBuffers(1, &vbo_tangents_id); }
}

/*! draws the model
 */
void a2estatic::draw(const DRAW_MODE draw_mode) {
	if(is_draw_phys_obj) {
		draw_phys_obj();
	}
	
	if(!is_draw_phys_obj && engine::get_init_mode() == engine::INIT_MODE::GRAPHICAL) {
		pre_draw_setup();
		
		// vbo setup, part one (the same for all sub-objects)
		draw_vertices_vbo = vbo_vertices_id;
		draw_tex_coords_vbo = vbo_tex_coords_id;
		draw_normals_vbo = vbo_normals_id;
		draw_binormals_vbo = vbo_binormals_id;
		draw_tangents_vbo = vbo_tangents_id;
		for(size_t i = 0; i < object_count; i++) {
			// vbo setup, part two
			draw_indices_vbo = vbo_indices_ids[i];
			draw_index_count = index_count[i] * 3;
			draw_sub_object(draw_mode, i, 0);
		}
		
		post_draw_setup();
	}
}

void a2estatic::pre_draw_setup(const ssize_t sub_object_num) {
	a2emodel::pre_draw_setup(sub_object_num);
	if(sub_object_num >= 0) {
		// vbo setup, part two
		draw_indices_vbo = vbo_indices_ids[sub_object_num];
		draw_index_count = index_count[sub_object_num] * 3;
	}
}

void a2estatic::post_draw_setup(const ssize_t sub_object_num) {
	a2emodel::post_draw_setup(sub_object_num);
}

/*! loads a .a2m model file
 *  @param filename the name of the .a2m model file
 *  @param vbo flag that specifies if vertex buffer objects should be used
 */
void a2estatic::load_model(const string& filename_) {
	file_io file(filename_, file_io::OPEN_TYPE::READ_BINARY);
	if(!file.is_open()) {
		return;
	}
	filename = filename_;

	// get type and name
	char* file_type = new char[9];
	file.get_block(file_type, 8);
	file_type[8] = 0;

	if(strcmp(file_type, "A2EMODEL") != 0) {
		log_error("non supported file type for %s: %s!", filename, file_type);
		delete [] file_type;
		file.close();
		return;
	}
	delete [] file_type;
	
	// get model version
	unsigned int version = file.get_uint();
	if(version != A2M_VERSION) {
		log_error("wrong model file version %u - should be %u!", version, A2M_VERSION);
		file.close();
		return;
	}

	// get model type and abort if it's not 0x00 or 0x02
	char mtype = file.get_char();
	if(mtype != 0x00 && mtype != 0x02) {
		log_error("non supported model type: %u!", (unsigned int)(mtype & 0xFF));
		file.close();
		return;
	}

	if(mtype == 0x02) collision_model = true;

	vertex_count = file.get_uint();
	tex_coord_count = file.get_uint();
	vertices = new float3[vertex_count];
	normals = new float3[vertex_count];
	binormals = new float3[vertex_count];
	tangents = new float3[vertex_count];
	tex_coords = new coord[tex_coord_count];

	for(unsigned int i = 0; i < vertex_count; i++) {
		vertices[i].x = file.get_float();
		vertices[i].y = file.get_float();
		vertices[i].z = file.get_float();
	}
	for(unsigned int i = 0; i < tex_coord_count; i++) {
		tex_coords[i].u = file.get_float();
		tex_coords[i].v = 1.0f - file.get_float();
	}

	object_count = file.get_uint();
	delete_sub_bboxes();
	sub_bboxes.resize(object_count);

	object_names.clear();
	object_names.resize(object_count);
	for(unsigned int i = 0; i < object_count; i++) {
		file.get_terminated_block(object_names[i], (char)0xFF);
	}

	indices = new index3*[object_count];
	tex_indices = new index3*[object_count];
	index_count = new unsigned int[object_count];
	min_index = new unsigned int[object_count];
	max_index = new unsigned int[object_count];
	memset(min_index, 0xFF, sizeof(unsigned int)*object_count);
	memset(max_index, 0, sizeof(unsigned int)*object_count);
	for(unsigned int i = 0; i < object_count; i++) {
		index_count[i] = file.get_uint();
		indices[i] = new index3[index_count[i]];
		tex_indices[i] = new index3[index_count[i]];
		for(unsigned int j = 0; j < index_count[i]; j++) {
			indices[i][j].x = file.get_uint();
			indices[i][j].y = file.get_uint();
			indices[i][j].z = file.get_uint();
		}
		for(unsigned int j = 0; j < index_count[i]; j++) {
			tex_indices[i][j].x = file.get_uint();
			tex_indices[i][j].y = file.get_uint();
			tex_indices[i][j].z = file.get_uint();
		}
	}

	if(collision_model) {
		col_vertex_count = file.get_uint();
		col_vertices = new float3[col_vertex_count];
		for(unsigned int i = 0; i < col_vertex_count; i++) {
			col_vertices[i].x = file.get_float();
			col_vertices[i].y = file.get_float();
			col_vertices[i].z = file.get_float();
		}

		col_index_count = file.get_uint();
		col_indices = new index3[col_index_count];
		for(unsigned int i = 0; i < col_index_count; i++) {
			col_indices[i].x = file.get_uint();
			col_indices[i].y = file.get_uint();
			col_indices[i].z = file.get_uint();
		}
	}

	file.close();
	
	// set this stuff for normal generating
	model_indices = indices;
	model_index_count = index_count;

	generate_normals();
	reorganize_model_data();
	
	// set the actual model data after everything is computed correctly and its final state
	model_indices = indices;
	model_index_count = index_count;
	
	model_vertices = new float3*[object_count];
	model_tex_coords = new coord*[object_count];
	model_vertex_count = new unsigned int[object_count];
	for(unsigned int i = 0; i < object_count; i++) {
		model_vertices[i] = &vertices[min_index[i]];
		model_tex_coords[i] = &tex_coords[min_index[i]];
		model_vertex_count[i] = max_index[i] - min_index[i] + 1;
	}

	build_bounding_box();
	
	// vertices vbo
	glGenBuffers(1, &vbo_vertices_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
	
	// tex_coords vbo
	glGenBuffers(1, &vbo_tex_coords_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_tex_coords_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 2 * sizeof(float), tex_coords, GL_STATIC_DRAW);
	
	// normals/binormals/tangents vbo
	glGenBuffers(1, &vbo_normals_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), normals, GL_STATIC_DRAW);
	
	glGenBuffers(1, &vbo_binormals_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_binormals_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), binormals, GL_STATIC_DRAW);
	
	glGenBuffers(1, &vbo_tangents_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_tangents_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), tangents, GL_STATIC_DRAW);
	
	// indices vbos
	vbo_indices_ids = new GLuint[object_count];
	for(unsigned int i = 0; i < object_count; i++) {
		glGenBuffers(1, &vbo_indices_ids[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices_ids[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count[i] * 3 * sizeof(unsigned int), indices[i], GL_STATIC_DRAW);
	}
	
	// reset buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	// general model setup
	model_setup();
}

void a2estatic::load_from_memory(unsigned int object_count_, unsigned int vertex_count_,
								 float3* vertices_, coord* tex_coords_,
								 unsigned int* index_count_, index3** indices_) {
	filename = "<memory>";
	a2estatic::vertex_count = vertex_count_;
	a2estatic::vertices = vertices_;
	
	a2estatic::tex_coord_count = vertex_count_;
	a2estatic::tex_coords = tex_coords_;
	
	a2estatic::index_count = index_count_;
	a2estatic::indices = indices_;
	a2estatic::tex_indices = indices_;
	normals = new float3[vertex_count];
	binormals = new float3[vertex_count];
	tangents = new float3[vertex_count];
	
	a2estatic::object_count = object_count_;
	min_index = new unsigned int[object_count];
	max_index = new unsigned int[object_count];
	memset(min_index, 0xFF, sizeof(unsigned int)*object_count);
	memset(max_index, 0, sizeof(unsigned int)*object_count);
	for(unsigned int i = 0; i < object_count; i++) {
		if(index_count[i] == 0) {
			min_index[i] = 0;
			max_index[i] = 0;
		}
		for(unsigned int j = 0; j < index_count[i]; j++) {			
			// also get the max/highest and min/lowest index number
			if(tex_indices[i][j].x > max_index[i]) max_index[i] = tex_indices[i][j].x;
			if(tex_indices[i][j].y > max_index[i]) max_index[i] = tex_indices[i][j].y;
			if(tex_indices[i][j].z > max_index[i]) max_index[i] = tex_indices[i][j].z;
			if(tex_indices[i][j].x < min_index[i]) min_index[i] = tex_indices[i][j].x;
			if(tex_indices[i][j].y < min_index[i]) min_index[i] = tex_indices[i][j].y;
			if(tex_indices[i][j].z < min_index[i]) min_index[i] = tex_indices[i][j].z;
		}
	}
	
	delete_sub_bboxes();
	sub_bboxes.resize(object_count);
	
	object_names.clear();
	object_names.resize(object_count);
	for(unsigned int i = 0; i < object_count; i++) {
		object_names[i] = "object #" + uint2string(i);
	}
	
	// set this stuff for normal generating
	model_indices = indices;
	model_index_count = index_count;
	
	generate_normals();
	
	// set the actual model data after everything is computed correctly and its final state
	model_indices = indices;
	model_index_count = index_count;
	
	model_vertices = new float3*[object_count];
	model_vertex_count = new unsigned int[object_count];
	for(unsigned int i = 0; i < object_count; i++) {
		model_vertices[i] = &vertices[min_index[i]];
		model_vertex_count[i] = (min_index[i] != 0xFFFFFFFF ? max_index[i] - min_index[i] + 1 : 0);
	}
	
	build_bounding_box();
	
	// vertices vbo
	glGenBuffers(1, &vbo_vertices_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
	
	// tex_coords vbo
	glGenBuffers(1, &vbo_tex_coords_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_tex_coords_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 2 * sizeof(float), tex_coords, GL_STATIC_DRAW);
	
	// normals/binormals/tangents vbo
	glGenBuffers(1, &vbo_normals_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), normals, GL_STATIC_DRAW);
	
	glGenBuffers(1, &vbo_binormals_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_binormals_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), binormals, GL_STATIC_DRAW);
	
	glGenBuffers(1, &vbo_tangents_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_tangents_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), tangents, GL_STATIC_DRAW);
	
	// indices vbos
	vbo_indices_ids = new GLuint[object_count];
	for(unsigned int i = 0; i < object_count; i++) {
		glGenBuffers(1, &vbo_indices_ids[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices_ids[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count[i] * 3 * sizeof(unsigned int), indices[i], GL_STATIC_DRAW);
	}
	
	// reset buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	// general model setup
	model_setup();
}

/*! reorganizes the model data, giving each texture coordinate an own vertex (generating more vertices so that
 *! we have an equal count of vertices and texture coordinates) and "merging" the indices so that we only have
 *! one index array (which is a requirement of opengl).
 */
void a2estatic::reorganize_model_data() {
	float3* old_vertices = vertices;
	float3* old_normals = normals;
	float3* old_binormals = binormals;
	float3* old_tangents = tangents;
	
	vertex_count = tex_coord_count;
	vertices = new float3[vertex_count];
	normals = new float3[vertex_count];
	binormals = new float3[vertex_count];
	tangents = new float3[vertex_count];
	
	for(unsigned int i = 0; i < object_count; i++) {
		for(unsigned int j = 0; j < index_count[i]; j++) {
			vertices[tex_indices[i][j].x] = old_vertices[indices[i][j].x];
			vertices[tex_indices[i][j].y] = old_vertices[indices[i][j].y];
			vertices[tex_indices[i][j].z] = old_vertices[indices[i][j].z];
			
			normals[tex_indices[i][j].x] = old_normals[indices[i][j].x];
			normals[tex_indices[i][j].y] = old_normals[indices[i][j].y];
			normals[tex_indices[i][j].z] = old_normals[indices[i][j].z];
			
			binormals[tex_indices[i][j].x] = old_binormals[indices[i][j].x];
			binormals[tex_indices[i][j].y] = old_binormals[indices[i][j].y];
			binormals[tex_indices[i][j].z] = old_binormals[indices[i][j].z];
			
			tangents[tex_indices[i][j].x] = old_tangents[indices[i][j].x];
			tangents[tex_indices[i][j].y] = old_tangents[indices[i][j].y];
			tangents[tex_indices[i][j].z] = old_tangents[indices[i][j].z];
			
			// also get the max/highest and min/lowest index number
			if(tex_indices[i][j].x > max_index[i]) max_index[i] = tex_indices[i][j].x;
			if(tex_indices[i][j].y > max_index[i]) max_index[i] = tex_indices[i][j].y;
			if(tex_indices[i][j].z > max_index[i]) max_index[i] = tex_indices[i][j].z;
			if(tex_indices[i][j].x < min_index[i]) min_index[i] = tex_indices[i][j].x;
			if(tex_indices[i][j].y < min_index[i]) min_index[i] = tex_indices[i][j].y;
			if(tex_indices[i][j].z < min_index[i]) min_index[i] = tex_indices[i][j].z;
		}
	}
	
	for(unsigned int i = 0; i < object_count; i++) {
		delete [] indices[i];
	}
	delete [] indices;
	indices = tex_indices;
}

/*! sets the "vertex scale" of the model (the model itself is scaled)
 *  @param x the x scale
 *  @param y the y scale
 *  @param z the z scale
 */
void a2estatic::set_hard_scale(const float x, const float y, const float z) {
	for(unsigned int i = 0; i < vertex_count; i++) {
		vertices[i].x *= x;
		vertices[i].y *= y;
		vertices[i].z *= z;
	}
	
	// reupload vertices stuff to vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * 3 * sizeof(float), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	if(collision_model) {
		for(unsigned int i = 0; i < col_vertex_count; i++) {
			col_vertices[i].x *= x;
			col_vertices[i].y *= y;
			col_vertices[i].z *= z;
		}
	}
	
	// rebuild the bounding box
	build_bounding_box();
}

void a2estatic::set_hard_position(const float x, const float y, const float z) {
	for(unsigned int i = 0; i < vertex_count; i++) {
		vertices[i].x += x;
		vertices[i].y += y;
		vertices[i].z += z;
	}
	
	// reupload vertices stuff to vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * 3 * sizeof(float), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	if(collision_model) {
		for(unsigned int i = 0; i < col_vertex_count; i++) {
			col_vertices[i].x += x;
			col_vertices[i].y += y;
			col_vertices[i].z += z;
		}
	}
	
	// rebuild the bounding box
	build_bounding_box();
}

/*! generates the normals, binormals and tangents of the model
 */
void a2estatic::generate_normals() {
	float3 normal, binormal, tangent;
	for(unsigned int i = 0; i < object_count; i++) {
		for(unsigned int j = 0; j < index_count[i]; j++) {
			core::compute_normal_tangent_binormal(vertices[indices[i][j].x],
												  vertices[indices[i][j].y],
												  vertices[indices[i][j].z],
												  normal, binormal, tangent,
												  tex_coords[tex_indices[i][j].x],
												  tex_coords[tex_indices[i][j].y],
												  tex_coords[tex_indices[i][j].z]);
			normals[indices[i][j].x] += normal;
			normals[indices[i][j].y] += normal;
			normals[indices[i][j].z] += normal;
			binormals[indices[i][j].x] += binormal;
			binormals[indices[i][j].y] += binormal;
			binormals[indices[i][j].z] += binormal;
			tangents[indices[i][j].x] += tangent;
			tangents[indices[i][j].y] += tangent;
			tangents[indices[i][j].z] += tangent;
		}
	}
	for(unsigned int i = 0; i < vertex_count; i++) {
		normals[i].normalize();
		binormals[i].normalize();
		tangents[i].normalize();
	}
}

/*! scales the texture coordinates by (su, sv) - note that this is a "hard scale" and the vbo is updated automatically
 *  @param su the su scale factor
 *  @param sv the sv scale factor
 */
void a2estatic::scale_tex_coords(const float su, const float sv) {
	for(unsigned int i = 0; i < vertex_count; i++) {
		tex_coords[i].u *= su;
		tex_coords[i].v *= sv;
	}
	
	// delete old vertex coordinates
	if(glIsBuffer(vbo_tex_coords_id)) { glDeleteBuffers(1, &vbo_tex_coords_id); }
	// create new buffer
	glGenBuffers(1, &vbo_tex_coords_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_tex_coords_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 2 * sizeof(float), tex_coords, GL_STATIC_DRAW);
}

/*! returns a pointer to the models collision model vertices
 */
float3* a2estatic::get_col_vertices() {
	return col_vertices;
}

/*! returns a pointer to the models collision model indices
 */
index3* a2estatic::get_col_indices() {
	return col_indices;
}

/*! returns the models collision model vertex count
 */
unsigned int a2estatic::get_col_vertex_count() {
	return col_vertex_count;
}

/*! returns the models collision model index count
 */
unsigned int a2estatic::get_col_index_count() {
	return col_index_count;
}
