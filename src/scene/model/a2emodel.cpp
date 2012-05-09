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

#include "a2emodel.h"
#include "scene/scene.h"

static size_t _initial_model_id = 0;
static size_t _create_model_id() {
	_initial_model_id+=4;
	return _initial_model_id;
}

/*! a2emodel constructor
 */
a2emodel::a2emodel(engine* e_, shader* s_, scene* sce_) {
	scale.set(1.0f, 1.0f, 1.0f);
	phys_scale.set(1.0f, 1.0f, 1.0f);
	rotation.set(0.0f, 0.0f, 0.0f);
	sub_bboxes = nullptr;
	
	model_vertices = nullptr;
	model_tex_coords = nullptr;
	model_indices = nullptr;
	model_vertex_count = nullptr;
	model_index_count = nullptr;
	object_count = 0;
	object_names = nullptr;
	
	collision_model = false;
	col_vertex_count = 0;
	col_index_count = 0;
	col_vertices = nullptr;
	col_indices = nullptr;
	
	draw_wireframe = false;
	is_visible = true;
	is_material = false;
	is_draw_phys_obj = false;
	is_transparent = false;
	
	radius = 0.0f;
	length = 0.0f;
	
	draw_vertices_vbo = 0;
	draw_tex_coords_vbo = 0;
	draw_normals_vbo = 0;
	draw_binormals_vbo = 0;
	draw_tangents_vbo = 0;
	draw_indices_vbo = 0;
	
	g_buffer = nullptr;
	l_buffer = nullptr;
	g_buffer_alpha = nullptr;
	l_buffer_alpha = nullptr;
	id = _create_model_id();
	
	// get classes
	a2emodel::e = e_;
	a2emodel::s = s_;
	a2emodel::sce = sce_;
	a2emodel::t = e->get_texman();
	a2emodel::exts = e->get_ext();
	a2emodel::ocl = e->get_opencl();
	a2emodel::material = nullptr;
}

/*! a2emodel destructor
 */
a2emodel::~a2emodel() {
	a2e_debug("deleting a2emodel object");
	
	if(object_names != nullptr) { delete [] object_names; }
	delete_sub_bboxes();
	
	is_sub_object_transparent.clear();
	
	a2e_debug("a2emodel object deleted");
}

void a2emodel::delete_sub_bboxes() {
	if(sub_bboxes != nullptr) {
		for(unsigned int i = 0; i < object_count; i++) {
			if(is_sub_object_transparent[i]) {
				sce->delete_alpha_object(&sub_bboxes[i]);
			}
		}
		delete [] sub_bboxes;
	}
}

void a2emodel::model_setup() {
	is_sub_object_transparent.resize(object_count);
	is_sub_object_transparent.assign(object_count, is_transparent);
}

void a2emodel::pre_draw_setup(const ssize_t sub_object_num) {
	// scale the model
	mvm = scale_mat;
	
	// rotate the model (use local inverse model view matrix that we already calculated)
	mvm *= mview_mat;
	
	// translate the model
	mvm *= *e->get_translation_matrix();
	
	// translate to model origin
	mvm *= matrix4f().translate(position.x, position.y, position.z);
	
	//
	mvm *= *e->get_rotation_matrix();
	mvpm = mvm * *e->get_projection_matrix();
	
	// if the wireframe flag is set, draw the model in wireframe mode
#if !defined(A2E_IOS)
	if(draw_wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
#endif
}

void a2emodel::post_draw_setup(const ssize_t sub_object_num) {
	// reset to filled mode
#if !defined(A2E_IOS)
	if(draw_wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#endif
}

/*! draws the model/object (all variables have to be set by the derived class beforehand)
 */
void a2emodel::draw_sub_object(const DRAW_MODE& draw_mode, const size_t& sub_object_num, const size_t& mask_id) {
	if(!(draw_mode >= DRAW_MODE::GEOMETRY_PASS &&
		 draw_mode <= DRAW_MODE::MATERIAL_ALPHA_PASS)) {
		a2e_error("invalid draw_mode: %u!", (unsigned int)draw_mode);
		return;
	}
	
	a2ematerial::MATERIAL_TYPE mat_type = material->get_material_type(sub_object_num);
	a2ematerial::LIGHTING_MODEL lm_type = material->get_lighting_model_type(sub_object_num);
	
	// inferred rendering
	size_t attr_array_mask = 0;
	size_t texture_mask = 0;
	
	// check mask id
	if(mask_id > A2E_MAX_MASK_ID) return;
	
	// check draw mode
	const bool transparent_sub_object = is_sub_object_transparent[sub_object_num];
	if(transparent_sub_object &&
	   draw_mode != DRAW_MODE::GEOMETRY_ALPHA_PASS &&
	   draw_mode != DRAW_MODE::MATERIAL_ALPHA_PASS) {
		return;
	}
	else if(!transparent_sub_object &&
			(draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS ||
			 draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS)) return;
	
	if(draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS ||
	   draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) {
		pre_draw_setup(sub_object_num);
	}
	
	// little hacky, but it's working for the moment :> (TODO: better method?)
	float2 model_id;
	model_id.x = id;
	model_id.y = sub_object_num*4;
	
	//
	gl3shader shd;
	const string shd_option = (draw_mode == DRAW_MODE::GEOMETRY_PASS ||
							   draw_mode == DRAW_MODE::MATERIAL_PASS ? "opaque" : "alpha");
	string shd_name = select_shader(draw_mode);
	if(shd_name != "") {
		shd = s->get_gl3shader(shd_name);
		shd->use(shd_option);
	}
	
	if(draw_mode == DRAW_MODE::GEOMETRY_PASS ||
	   draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS) {
		if(shd_name == "") {
			// first, select shader dependent on material type
			switch(mat_type) {
				// parallax mapping
				case a2ematerial::PARALLAX: {
					shd_name = material->is_parallax_occlusion(sub_object_num) ? "IR_GP_GBUFFER_PARALLAX" : "IR_GP_GBUFFER_PARALLAX";
					
					shd = s->get_gl3shader(shd_name);
					shd->use(shd_option);
					shd->uniform("cam_position", -float3(*e->get_position()));
					shd->uniform("model_position", position);
					
					attr_array_mask |= VA_TEXTURE_COORD | VA_BINORMAL | VA_TANGENT;
					texture_mask |= a2ematerial::TT_NORMAL | a2ematerial::TT_HEIGHT;
				}
				break;
				// diffuse mapping
				case a2ematerial::DIFFUSE:
				default: {
					shd = s->get_gl3shader("IR_GP_GBUFFER");
					shd->use(shd_option);
				}
				break;
			}
		}
		
		switch(lm_type) {
			// ward lighting
			case a2ematerial::LM_WARD: {
				const a2ematerial::ward_model* ward_lm = (const a2ematerial::ward_model*)material->get_lighting_model(sub_object_num);
				shd->uniform("Nuv", ward_lm->isotropic_roughness, ward_lm->isotropic_roughness);
			}
			break;
			// ashikhmin/shirley lighting
			case a2ematerial::LM_ASHIKHMIN_SHIRLEY: {
				const a2ematerial::ashikhmin_shirley_model* aslm = (const a2ematerial::ashikhmin_shirley_model*)material->get_lighting_model(sub_object_num);
				shd->uniform("Nuv", aslm->anisotropic_roughness.u, aslm->anisotropic_roughness.v);
			}
			break;
			// phong lighting
			case a2ematerial::LM_PHONG:
			default:
				shd->uniform("Nuv", 16.0f, 16.0f);
				break;
		}
		
		attr_array_mask |= VA_NORMAL;
		
		// custom pre-draw setup
		pre_draw_geometry(shd, attr_array_mask, texture_mask);
	}
	else if(draw_mode == DRAW_MODE::MATERIAL_PASS ||
			draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) {
		attr_array_mask |= VA_TEXTURE_COORD;
		texture_mask |= a2ematerial::TT_DIFFUSE | a2ematerial::TT_SPECULAR;
		
		if(shd_name == "") {
			// first, select shader dependent on material type
			switch(mat_type) {
				// parallax mapping
				case a2ematerial::PARALLAX: {
					shd_name = material->is_parallax_occlusion(sub_object_num) ? "IR_MP_PARALLAX" : "IR_MP_PARALLAX";
					
					shd = s->get_gl3shader(shd_name);
					shd->use(shd_option);
					shd->uniform("cam_position", -float3(*e->get_position()));
					shd->uniform("model_position", position);
					
					attr_array_mask |= VA_NORMAL | VA_BINORMAL | VA_TANGENT;
					texture_mask |= a2ematerial::TT_HEIGHT;
				}
				break;
				// diffuse mapping
				case a2ematerial::DIFFUSE:
				default: {
					shd = s->get_gl3shader("IR_MP_DIFFUSE");
					shd->use(shd_option);
				}
				break;
			}
		}
		
		// inferred rendering setup
		ir_mp_setup(shd, shd_option);
		
		// custom pre-draw setup
		pre_draw_material(shd, attr_array_mask, texture_mask);
	}
	if(draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS ||
	   draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) {
		shd->uniform("mask_id", (float)mask_id);
		shd->uniform("id", model_id);
	}
	
	if(attr_array_mask & VA_NORMAL) shd->uniform("local_mview", mview_mat);
	if(attr_array_mask & VA_NORMAL) shd->uniform("local_scale", scale_mat);
	
	//
	material->enable_textures(sub_object_num, shd, texture_mask);
	
	shd->uniform("mvpm", mvpm);
	
	shd->attribute_array("in_vertex", draw_vertices_vbo, 3);
	if(attr_array_mask & VA_NORMAL) shd->attribute_array("normal", draw_normals_vbo, 3);
	if(attr_array_mask & VA_TEXTURE_COORD) shd->attribute_array("texture_coord", draw_tex_coords_vbo, 2);
	if(attr_array_mask & VA_BINORMAL) shd->attribute_array("binormal", draw_binormals_vbo, 3);
	if(attr_array_mask & VA_TANGENT) shd->attribute_array("tangent", draw_tangents_vbo, 3);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_indices_vbo);
	glDrawElements(GL_TRIANGLES, (GLsizei)draw_index_count, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	material->disable_textures(sub_object_num);
	
	if(draw_mode == DRAW_MODE::GEOMETRY_PASS ||
	   draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS) {
		// custom post-draw setup
		post_draw_geometry(shd);
	}
	else if(draw_mode == DRAW_MODE::MATERIAL_PASS ||
			draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) {
		// custom post-draw setup
		post_draw_material(shd);
	}
	
	shd->disable();

	if(draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS ||
	   draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) {
		post_draw_setup(sub_object_num);
	}
}

void a2emodel::ir_mp_setup(gl3shader& shd, const string& option) {
	const rtt::fbo* cur_buffer = e->get_rtt()->get_current_buffer();
	const float2 screen_size = float2(float(cur_buffer->width), float(cur_buffer->height));
	shd->uniform("screen_size", screen_size);
	
	if(option == "opaque") {
		shd->texture("light_buffer_diffuse", l_buffer->tex_id[0]);
		shd->texture("light_buffer_specular", l_buffer->tex_id[1]);
	}
	else if(option == "alpha") {
		shd->texture("light_buffer_diffuse", l_buffer->tex_id[0]);
		shd->texture("light_buffer_specular", l_buffer->tex_id[1]);
		
		// global mvm is currently only used in the material alpha pass
		shd->uniform("mvm", mvm);
		
		// compute projection constants (necessary to reconstruct world pos)
		const float2 near_far_plane = e->get_near_far_plane();
		const float2 projection_ab = float2(near_far_plane.y / (near_far_plane.y - near_far_plane.x),
											(-near_far_plane.y * near_far_plane.x) / (near_far_plane.y - near_far_plane.x));
		shd->uniform("projection_ab", projection_ab);
		
		const float2 l_buffer_size = float2(float(l_buffer_alpha->width), float(l_buffer_alpha->height));
		shd->uniform("l_buffer_size", l_buffer_size);
		shd->uniform("texel_size", float2(1.0f)/l_buffer_size);
		
		shd->texture("dsf_buffer", g_buffer_alpha->tex_id[1]);
		shd->texture("depth_buffer", g_buffer_alpha->depth_buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
}

void a2emodel::pre_draw_geometry(gl3shader& shd, size_t& attr_array_mask, size_t& texture_mask) {
}

void a2emodel::post_draw_geometry(gl3shader& shd) {
}

void a2emodel::pre_draw_material(gl3shader& shd, size_t& attr_array_mask, size_t& texture_mask) {
}

void a2emodel::post_draw_material(gl3shader& shd) {
}

const string a2emodel::select_shader(const DRAW_MODE& draw_mode) const {
	return "";
}

/*! draws the models physical model/object
 */
void a2emodel::draw_phys_obj() {
	// to be overwritten
}

/*! sets the position of the model
 *  @param x the x coordinate
 *  @param y the y coordinate
 *  @param z the z coordinate
 */
void a2emodel::set_position(float x, float y, float z) {
	position.set(x, y, z);
	
	bbox.pos.set(x, y, z);
	for(unsigned int i = 0; i < object_count; i++) {
		sub_bboxes[i].pos.set(x, y, z);
	}
}

/*! sets the position of the model
 *  @param pos the new position
 */
void a2emodel::set_position(float3* pos) {
	set_position(pos->x, pos->y, pos->z);
}

/*! sets the rotation of the model
 *  @param x the x rotation
 *  @param y the y rotation
 *  @param z the z rotation
 */
void a2emodel::set_rotation(float x, float y, float z) {
	rotation.set(x, y, z);
	
	update_mview_matrix();
	
	// update bounding boxes mview matrix
	bbox.mview = mview_mat;
	for(unsigned int i = 0; i < object_count; i++) {
		sub_bboxes[i].mview = mview_mat;
	}
}

/*! sets the rotation of the model
 *  @param pos the new rotation
 */
void a2emodel::set_rotation(float3* rot) {
	set_rotation(rot->x, rot->y, rot->z);
}

/*! sets the render scale of the model (the rendered model is scaled)
 *  @param x the x scale
 *  @param y the y scale
 *  @param z the z scale
 */
void a2emodel::set_scale(float x, float y, float z) {
	scale.set(x, y, z);
	
	update_scale_matrix();
	
	// rebuild the bounding box
	build_bounding_box();
	
	/*// TODO: update bounding boxes mview matrix
	bbox.mview = mview_mat;
	for(unsigned int i = 0; i < object_count; i++) {
		sub_bboxes[i].mview = mview_mat;
	}*/
}

/*! sets the scale of the model
 *  @param scl the new scale
 */
void a2emodel::set_scale(float3* scl) {
	set_scale(scl->x, scl->y, scl->z);
}

/*! sets the "vertex scale" of the model (the model itself is scaled)
 *  @param x the x scale
 *  @param y the y scale
 *  @param z the z scale
 */
void a2emodel::set_hard_scale(float x, float y, float z) {
	// to be overwritten
}

/*! sets the hard scale of the model
 *  @param hscl the new hard scale
 */
void a2emodel::set_hard_scale(float3* hscl) {
	set_hard_scale(hscl->x, hscl->y, hscl->z);
}

/*! sets the "vertex positions" of the model (the model itself is relocated)
 *  @param x the x position
 *  @param y the y position
 *  @param z the z position
 */
void a2emodel::set_hard_position(float x, float y, float z) {
	// to be overwritten
}

/*! sets the hard position of the model
 *  @param hpos the new hard position
 */
void a2emodel::set_hard_position(float3* hpos) {
	set_hard_scale(hpos->x, hpos->y, hpos->z);
}

/*! scales the texture coordinates by (su, sv) - note that this is a "hard scale" and the vbo is updated automatically
 *  @param su the su scale factor
 *  @param sv the sv scale factor
 */
void a2emodel::scale_tex_coords(float su, float sv) {
	// to be overwritten
}

/*! returns the position of the model
 */
float3* a2emodel::get_position() {
	return &position;
}

/*! returns the scale of the model
 */
float3* a2emodel::get_scale() {
	return &scale;
}

/*! returns the rotation of the model
 */
float3* a2emodel::get_rotation() {
	return &rotation;
}

void a2emodel::set_mview_matrix(const matrix4f& mat) {
	mview_mat = mat;
}

/*! updates the local modelview matrix
 */
void a2emodel::update_mview_matrix() {
	// TODO: fix parallax mapping ...
	mview_mat = matrix4f().rotate_x(rotation.x) * matrix4f().rotate_y(rotation.y) * matrix4f().rotate_z(rotation.z);
	mview_mat.invert();
}

/*! updates the local scale matrix
 */
void a2emodel::update_scale_matrix() {
	scale_mat.identity();
	scale_mat.scale(scale.x, scale.y, scale.z);
}

/*! builds the bounding box
 */
void a2emodel::build_bounding_box() {
	float3 min(model_vertices[0][0]), max(model_vertices[0][0]);
	for(unsigned int i = 0; i < object_count; i++) {
		float3 smin(model_vertices[i][0]), smax(model_vertices[i][0]);
		extbbox& sbbox = sub_bboxes[i];
		
		for(unsigned int j = 0; j < model_vertex_count[i]; j++) {
			const float3& vert = model_vertices[i][j];
			smin = float3::min(smin, vert);
			smax = float3::max(smax, vert);
		}
		
		min = float3::min(smin, min);
		max = float3::max(smax, max);
		
		sbbox.min = smin;
		sbbox.max = smax;
		sbbox.min.scale(scale);
		sbbox.max.scale(scale);
		sbbox.mview = mview_mat;
		sbbox.pos.set(position);
	}
	
	bbox.min = min;
	bbox.max = max;
	
	// scale bbox
	bbox.min.scale(scale);
	bbox.max.scale(scale);
	
	// rotate bbox
	bbox.mview = mview_mat;
	
	// set bbox position
	bbox.pos.set(position);
}

/*! returns the bounding box of the model
 */
extbbox* a2emodel::get_bounding_box() {
	return &(a2emodel::bbox);
}

/*! returns the bounding box of the model
 */
extbbox* a2emodel::get_bounding_box(const size_t& sub_object) {
	if(sub_bboxes == nullptr || sub_object >= object_count) return nullptr;
	return &sub_bboxes[sub_object];
}

/*! sets the models draw physical object state
 *  @param state the new state
 */
void a2emodel::set_draw_phys_obj(bool state) {
	a2emodel::is_draw_phys_obj = state;
}

/*! sets the bool if the model is drawn as a wireframe
 *  @param state the new state
 */
void a2emodel::set_draw_wireframe(bool state) {
	a2emodel::draw_wireframe = state;
}

/*! sets the models visible flag
 *  @param state the new state
 */
void a2emodel::set_visible(bool state) {
	a2emodel::is_visible = state;
}

/*! returns the models draw physical object flag
 */
bool a2emodel::get_draw_phys_obj() {
	return a2emodel::is_draw_phys_obj;
}

/*! returns a true if the model is drawn as a wireframe
 */
bool a2emodel::get_draw_wireframe() {
	return a2emodel::draw_wireframe;
}

/*! returns a true if the model is visible
 */
bool a2emodel::get_visible() {
	return a2emodel::is_visible;
}

/*! returns true if the model has a collision model
 */
bool a2emodel::is_collision_model() {
	return a2emodel::collision_model;
}

/*! sets the models material
 *  @param material the material object that we want tu use
 */
void a2emodel::set_material(a2ematerial* material_) {
	a2emodel::material = material_;
	a2emodel::is_material = true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// model data functions                                                                                    ////

/*! returns a pointer to the vertices
 */
float3** a2emodel::get_vertices() {
	return model_vertices;
}

/*! returns a pointer to the vertices
 */
float3* a2emodel::get_vertices(unsigned int obj_num) {
	return model_vertices[obj_num];
}

/*! returns a pointer to the tex coords
 */
coord** a2emodel::get_tex_coords() {
	return model_tex_coords;
}

/*! returns a pointer to the tex coords
 */
coord* a2emodel::get_tex_coords(unsigned int obj_num) {
	return model_tex_coords[obj_num];
}

/*! returns a pointer to all the indices
 */
index3** a2emodel::get_indices() {
	return model_indices;
}

/*! returns a pointer to the specified (by obj_num) indices
 *  @param obj_num sub-object number we want the indices from
 */
index3* a2emodel::get_indices(unsigned int obj_num) {
	return model_indices[obj_num];
}

/*! returns the vertex count
 */
unsigned int a2emodel::get_vertex_count() {
	unsigned int model_total_vertex_count = 0;
	for(unsigned int i = 0; i < object_count; i++) {
		model_total_vertex_count += model_vertex_count[i];
	}
	return model_total_vertex_count;
}

/*! returns the vertex count of the specified sub-object
 */
unsigned int a2emodel::get_vertex_count(unsigned int obj_num) {
	return model_vertex_count[obj_num];
}

/*! returns the index count
 */
unsigned int a2emodel::get_index_count() {
	unsigned int model_total_index_count = 0;
	for(unsigned int i = 0; i < object_count; i++) {
		model_total_index_count += model_index_count[i];
	}
	return model_total_index_count;
}

/*! returns the index count of the sub-object obj_num
 *  @param obj_num sub-object we want to get the index count from
 */
unsigned int a2emodel::get_index_count(unsigned int obj_num) {
	return model_index_count[obj_num];
}

/*! returns a string pointer to the models sub-object names
 */
string* a2emodel::get_object_names() {
	return a2emodel::object_names;
}

/*! returns the models sub-object count
 */
unsigned int a2emodel::get_object_count() {
	return a2emodel::object_count;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// stuff for collision detection                                                                           ////

/*! used for ode collision - sets the radius of an sphere/cylinder object
 *  @param radius the objects radius
 */
void a2emodel::set_radius(float radius_) {
	a2emodel::radius = radius_;
}

/*! used for ode collision - sets the length of an cylinder object
 *  @param length the objects length
 */
void a2emodel::set_length(float length_) {
	a2emodel::length = length_;
}

/*! used for ode collision - returns the radius of an sphere/cylinder object
 */
float a2emodel::get_radius() {
	return a2emodel::radius;
}

/*! used for ode collision - returns the length of an cylinder object
 */
float a2emodel::get_length() {
	return a2emodel::length;
}

/*! sets the models physical scale
 *  @param x x scale
 *  @param y y scale
 *  @param z z scale
 */
void a2emodel::set_phys_scale(float x, float y, float z) {
	phys_scale.set(x, y, z);
}

/*! sets the models physical scale
 *  @param pscl the new physical scale
 */
void a2emodel::set_phys_scale(float3* pscl) {
	set_phys_scale(pscl->x, pscl->y, pscl->z);
}

/*! returns the physical scale
 */
float3* a2emodel::get_phys_scale() {
	return &phys_scale;
}

/*! returns a pointer to the models collision model vertices
 */
float3* a2emodel::get_col_vertices() {
	return col_vertices;
}

/*! returns a pointer to the models collision model indices
 */
index3* a2emodel::get_col_indices() {
	return col_indices;
}

/*! returns the models collision model vertex count
 */
unsigned int a2emodel::get_col_vertex_count() {
	return col_vertex_count;
}

/*! returns the models collision model index count
 */
unsigned int a2emodel::get_col_index_count() {
	return col_index_count;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// lighting stuff                                                                                          ////

// inferred rendering
void a2emodel::set_ir_buffers(const rtt::fbo* g_buffer_,
							  const rtt::fbo* l_buffer_,
							  const rtt::fbo* g_buffer_alpha_,
							  const rtt::fbo* l_buffer_alpha_) {
	a2emodel::g_buffer = g_buffer_;
	a2emodel::l_buffer = l_buffer_;
	a2emodel::g_buffer_alpha = g_buffer_alpha_;
	a2emodel::l_buffer_alpha = l_buffer_alpha_;
}

void a2emodel::set_transparent(const bool state) {
	is_transparent = state;
	is_sub_object_transparent.assign(object_count, state);
	
	// TODO: handle full-object transparency (remove all transparent sub-objs from scene-alpha-cb, add full object cb)
}

bool a2emodel::get_transparent() const {
	return is_transparent;
}

void a2emodel::set_transparent(const size_t& sub_object, const bool state) {
	if(sub_object >= is_sub_object_transparent.size()) return;
	
	if(is_sub_object_transparent[sub_object] != state) {
		if(state) {
			// TODO: handle draw_sub_object is function is overwritten
			draw_callback* cb = new draw_callback(this, &a2emodel::draw_sub_object);
			transparency_callbacks.push_back(cb);
			sce->add_alpha_object(&sub_bboxes[sub_object], sub_object, cb);
		}
		else {
			sce->delete_alpha_object(&sub_bboxes[sub_object]);
		}
	}
	
	is_sub_object_transparent[sub_object] = state;
}

bool a2emodel::get_transparent(const size_t& sub_object) const {
	if(sub_object >= is_sub_object_transparent.size()) return false;
	return is_sub_object_transparent[sub_object];
}
