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

#include "a2emodel.hpp"
#include "scene/scene.hpp"
#include <floor/math/quaternion.hpp>

static size_t _initial_model_id = 0;
static size_t _create_model_id() {
	_initial_model_id+=4;
	return _initial_model_id;
}

/*! a2emodel constructor
 */
a2emodel::a2emodel(shader* s_, scene* sce_) {
	scale.set(1.0f, 1.0f, 1.0f);
	phys_scale.set(1.0f, 1.0f, 1.0f);
	
	model_vertices = nullptr;
	model_tex_coords = nullptr;
	model_indices = nullptr;
	model_vertex_count = nullptr;
	model_index_count = nullptr;
	object_count = 0;
	
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
	a2emodel::s = s_;
	a2emodel::sce = sce_;
	a2emodel::t = engine::get_texman();
	a2emodel::exts = engine::get_ext();
	a2emodel::material = nullptr;
}

/*! a2emodel destructor
 */
a2emodel::~a2emodel() {
	delete_sub_bboxes();
	is_sub_object_transparent.clear();
}

void a2emodel::delete_sub_bboxes() {
	if(sub_bboxes.empty()) return;
	for(unsigned int i = 0; i < object_count; i++) {
		if(is_sub_object_transparent[i]) {
			sce->delete_alpha_object(&sub_bboxes[i]);
		}
	}
	sub_bboxes.clear();
}

const string& a2emodel::get_filename() const {
	return filename;
}

void a2emodel::model_setup() {
	is_sub_object_transparent.resize(object_count);
	is_sub_object_transparent.assign(object_count, is_transparent);
}

void a2emodel::pre_draw_setup(const ssize_t sub_object_num floor_unused) {
	// scale the model
	mvm = scale_mat;
	
	// rotate the model (use local inverse model view matrix that we already calculated)
	mvm *= rot_mat;
	
	// translate the model
	mvm *= *engine::get_translation_matrix();
	
	// translate to model origin
	mvm *= matrix4f().translate(position.x, position.y, position.z);
	
	// compute backside mvm/mvpm
	quaternionf q_x, q_y;
	q_x.set_rotation(engine::get_rotation()->x, float3(1.0f, 0.0f, 0.0f));
	q_y.set_rotation(180.0f - engine::get_rotation()->y, float3(0.0f, 1.0f, 0.0f));
	q_y *= q_x;
	q_y.normalize();
	mvpm_backside = mvm * q_y.to_matrix4();
	
	//
	mvm *= *engine::get_rotation_matrix();
	
	mvpm = mvm * *engine::get_projection_matrix();
	mvpm_backside = mvpm_backside * *engine::get_projection_matrix();
	
	// if the wireframe flag is set, draw the model in wireframe mode
#if !defined(FLOOR_IOS)
	if(draw_wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
#endif
}

void a2emodel::post_draw_setup(const ssize_t sub_object_num floor_unused) {
	// reset to filled mode
#if !defined(FLOOR_IOS)
	if(draw_wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#endif
}

/*! draws the model/object (all variables have to be set by the derived class beforehand)
 */
void a2emodel::draw_sub_object(const DRAW_MODE& draw_mode, const size_t& sub_object_num, const size_t& mask_id) {
	if(draw_mode == DRAW_MODE::NONE ||
	   draw_mode > DRAW_MODE::ENV_GM_PASSES_MASK) {
		log_error("invalid draw_mode: %u!", draw_mode);
		return;
	}
	const DRAW_MODE masked_draw_mode(draw_mode & DRAW_MODE::GM_PASSES_MASK);
	const bool env_pass((draw_mode & DRAW_MODE::ENVIRONMENT_PASS) != DRAW_MODE::NONE);
	
	a2ematerial::MATERIAL_TYPE mat_type = material->get_material_type(sub_object_num);
	a2ematerial::LIGHTING_MODEL lm_type = material->get_lighting_model_type(sub_object_num);
	
	// inferred rendering
	VERTEX_ATTRIBUTE attr_array_mask { (VERTEX_ATTRIBUTE)0 };
	a2ematerial::TEXTURE_TYPE texture_mask { (a2ematerial::TEXTURE_TYPE)0 };
	
	// check mask id
	if(mask_id > A2E_MAX_MASK_ID) return;
	
	// check draw mode
	const bool transparent_sub_object = is_sub_object_transparent[sub_object_num];
	if(transparent_sub_object &&
	   masked_draw_mode != DRAW_MODE::GEOMETRY_ALPHA_PASS &&
	   masked_draw_mode != DRAW_MODE::MATERIAL_ALPHA_PASS) {
		return;
	}
	else if(!transparent_sub_object &&
			(masked_draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS ||
			 masked_draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS)) return;
	
	if(masked_draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS ||
	   masked_draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) {
		pre_draw_setup((ssize_t)sub_object_num);
	}
	
	// little hacky, but it's working for the moment :> (TODO: better method?)
	float2 model_id;
	model_id.x = id;
	model_id.y = sub_object_num*4;
	
	//
	gl_shader shd;
	const bool has_env_map(env_map != 0);
	const string shd_option = (masked_draw_mode == DRAW_MODE::GEOMETRY_PASS ||
							   masked_draw_mode == DRAW_MODE::MATERIAL_PASS ?
							   "opaque" : "alpha");
	set<string> shd_combiners;
	if(env_pass) shd_combiners.insert("*env_probe");
	if((masked_draw_mode == DRAW_MODE::MATERIAL_PASS ||
		masked_draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) &&
	   has_env_map) {
		shd_combiners.insert("*env_map");
	}
	if((masked_draw_mode == DRAW_MODE::GEOMETRY_PASS ||
	   masked_draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS) &&
	   lm_type == a2ematerial::LIGHTING_MODEL::ASHIKHMIN_SHIRLEY) {
		const a2ematerial::ashikhmin_shirley_model* aslm = (const a2ematerial::ashikhmin_shirley_model*)material->get_lighting_model(sub_object_num);
		if(aslm->anisotropic_texture != nullptr) {
			shd_combiners.insert("*aux_texture");
		}
	}
	
	string shd_name = select_shader(draw_mode);
	if(shd_name != "") {
		shd = s->get_gl_shader(shd_name);
		shd->use(shd_option, shd_combiners);
	}
	
	if(masked_draw_mode == DRAW_MODE::GEOMETRY_PASS ||
	   masked_draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS) {
		if(shd_name == "") {
			// first, select shader dependent on material type
			switch(mat_type) {
				// parallax mapping
				case a2ematerial::MATERIAL_TYPE::PARALLAX: {
					shd_name = material->is_parallax_occlusion(sub_object_num) ? "IR_GP_GBUFFER_PARALLAX" : "IR_GP_GBUFFER_PARALLAX";
					
					shd = s->get_gl_shader(shd_name);
					shd->use(shd_option, shd_combiners);
					shd->uniform("cam_position", -float3(*engine::get_position()));
					shd->uniform("model_position", position);
					
					attr_array_mask |= VERTEX_ATTRIBUTE::TEXTURE_COORD | VERTEX_ATTRIBUTE::BINORMAL | VERTEX_ATTRIBUTE::TANGENT;
					texture_mask |= a2ematerial::TEXTURE_TYPE::NORMAL | a2ematerial::TEXTURE_TYPE::HEIGHT;
				}
				break;
				// diffuse mapping
				case a2ematerial::MATERIAL_TYPE::DIFFUSE:
				case a2ematerial::MATERIAL_TYPE::NONE: {
					shd = s->get_gl_shader("IR_GP_GBUFFER");
					shd->use(shd_option, shd_combiners);
				}
				break;
			}
		}
		
		switch(lm_type) {
			// ashikhmin/shirley lighting
			case a2ematerial::LIGHTING_MODEL::ASHIKHMIN_SHIRLEY: {
				const a2ematerial::ashikhmin_shirley_model* aslm = (const a2ematerial::ashikhmin_shirley_model*)material->get_lighting_model(sub_object_num);
				if(aslm->anisotropic_texture != nullptr) {
					shd->texture("aux_texture", aslm->anisotropic_texture);
				}
				else shd->uniform("Nuv", aslm->anisotropic_roughness.x, aslm->anisotropic_roughness.y);
			}
			break;
			// phong lighting
			case a2ematerial::LIGHTING_MODEL::PHONG:
			case a2ematerial::LIGHTING_MODEL::NONE:
				shd->uniform("Nuv", 16.0f, 16.0f);
				break;
		}
		
		attr_array_mask |= VERTEX_ATTRIBUTE::NORMAL;
		
		// custom pre-draw setup
		pre_draw_geometry(shd, attr_array_mask, texture_mask);
	}
	else if(masked_draw_mode == DRAW_MODE::MATERIAL_PASS ||
			masked_draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) {
		attr_array_mask |= VERTEX_ATTRIBUTE::TEXTURE_COORD;
		texture_mask |= a2ematerial::TEXTURE_TYPE::DIFFUSE | a2ematerial::TEXTURE_TYPE::SPECULAR | a2ematerial::TEXTURE_TYPE::REFLECTANCE;
		
		if(shd_name == "") {
			// first, select shader dependent on material type
			switch(mat_type) {
				// parallax mapping
				case a2ematerial::MATERIAL_TYPE::PARALLAX: {
					shd_name = material->is_parallax_occlusion(sub_object_num) ? "IR_MP_PARALLAX" : "IR_MP_PARALLAX";
					
					shd = s->get_gl_shader(shd_name);
					shd->use(shd_option, shd_combiners);
					shd->uniform("cam_position", -float3(*engine::get_position()));
					shd->uniform("model_position", position);
					
					attr_array_mask |= VERTEX_ATTRIBUTE::NORMAL | VERTEX_ATTRIBUTE::BINORMAL | VERTEX_ATTRIBUTE::TANGENT;
					texture_mask |= a2ematerial::TEXTURE_TYPE::HEIGHT;
				}
				break;
				// diffuse mapping
				case a2ematerial::MATERIAL_TYPE::DIFFUSE:
				case a2ematerial::MATERIAL_TYPE::NONE: {
					shd = s->get_gl_shader("IR_MP_DIFFUSE");
					shd->use(shd_option, shd_combiners);
				}
				break;
			}
		}
		
		// inferred rendering setup
		ir_mp_setup(shd, shd_option, shd_combiners);
		
		// custom pre-draw setup
		pre_draw_material(shd, attr_array_mask, texture_mask);
		
		if(has_env_map) {
			shd->texture("environment_map", env_map);
		}
	}
	if(masked_draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS ||
	   masked_draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) {
		shd->uniform("mask_id", (float)mask_id);
		shd->uniform("id", model_id);
	}
	
	if((unsigned int)(attr_array_mask & VERTEX_ATTRIBUTE::NORMAL) != 0) shd->uniform("local_mview", rot_mat);
	if((unsigned int)(attr_array_mask & VERTEX_ATTRIBUTE::NORMAL) != 0) shd->uniform("local_scale", scale_mat);
	
	//
	material->enable_textures(sub_object_num, shd, texture_mask);
	
	shd->uniform("mvpm", mvpm);
	if(env_pass) {
		shd->uniform("mvpm_backside", mvpm_backside);
	}
	
	shd->attribute_array("in_vertex", draw_vertices_vbo, 3);
	if((unsigned int)(attr_array_mask & VERTEX_ATTRIBUTE::NORMAL) != 0) shd->attribute_array("normal", draw_normals_vbo, 3);
	if((unsigned int)(attr_array_mask & VERTEX_ATTRIBUTE::TEXTURE_COORD) != 0) shd->attribute_array("texture_coord", draw_tex_coords_vbo, 2);
	if((unsigned int)(attr_array_mask & VERTEX_ATTRIBUTE::BINORMAL) != 0) shd->attribute_array("binormal", draw_binormals_vbo, 3);
	if((unsigned int)(attr_array_mask & VERTEX_ATTRIBUTE::TANGENT) != 0) shd->attribute_array("tangent", draw_tangents_vbo, 3);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_indices_vbo);
	glDrawElements(GL_TRIANGLES, (GLsizei)draw_index_count, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	material->disable_textures(sub_object_num);
	
	if(masked_draw_mode == DRAW_MODE::GEOMETRY_PASS ||
	   masked_draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS) {
		// custom post-draw setup
		post_draw_geometry(shd);
	}
	else if(masked_draw_mode == DRAW_MODE::MATERIAL_PASS ||
			masked_draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) {
		// custom post-draw setup
		post_draw_material(shd);
	}
	
	shd->disable();

	if(masked_draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS ||
	   masked_draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) {
		post_draw_setup((ssize_t)sub_object_num);
	}
}

void a2emodel::ir_mp_setup(gl_shader& shd, const string& option, const set<string>& combiners) {
	const rtt::fbo* cur_buffer = engine::get_rtt()->get_current_buffer();
	const float2 screen_size = float2(float(cur_buffer->width), float(cur_buffer->height));
	
	if(option == "opaque") {
		shd->texture("light_buffer_diffuse", l_buffer->tex[0]);
		shd->texture("light_buffer_specular", l_buffer->tex[1]);
	}
	else if(option == "alpha") {
		shd->uniform("screen_size", screen_size); // TODO: remove this in shader
		shd->texture("light_buffer_diffuse", l_buffer->tex[0]);
		shd->texture("light_buffer_specular", l_buffer->tex[1]);
		
		// global mvm is currently only used in the material alpha pass
		shd->uniform("mvm", mvm);
		
		// compute projection constants (necessary to reconstruct world pos)
		const float2 near_far_plane = floor::get_near_far_plane();
		const float2 projection_ab = float2(near_far_plane.y / (near_far_plane.y - near_far_plane.x),
											(-near_far_plane.y * near_far_plane.x) / (near_far_plane.y - near_far_plane.x));
		shd->uniform("projection_ab", projection_ab);
		
		const float2 l_buffer_size = float2(float(l_buffer_alpha->width), float(l_buffer_alpha->height));
		shd->uniform("l_buffer_size", l_buffer_size);
		shd->uniform("texel_size", float2(1.0f)/l_buffer_size);
		
		shd->texture("dsf_buffer", g_buffer_alpha->tex[1]);
		shd->texture("depth_buffer", g_buffer_alpha->depth_buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
	
	if(combiners.count("*env_map") > 0) {
		shd->uniform("local_mview", rot_mat);
		shd->uniform("local_scale", scale_mat);
		shd->uniform("model_position", position);
		shd->uniform("cam_position", -float3(*engine::get_position()));
		if(option == "opaque") {
			shd->texture("normal_buffer", g_buffer->tex[0]);
		}
		else if(option == "alpha") {
			shd->texture("normal_buffer", g_buffer_alpha->tex[0]);
		}
	}
}

void a2emodel::pre_draw_geometry(gl_shader& shd floor_unused, VERTEX_ATTRIBUTE& attr_array_mask floor_unused, a2ematerial::TEXTURE_TYPE& texture_mask floor_unused) {
}

void a2emodel::post_draw_geometry(gl_shader& shd floor_unused) {
}

void a2emodel::pre_draw_material(gl_shader& shd floor_unused, VERTEX_ATTRIBUTE& attr_array_mask floor_unused, a2ematerial::TEXTURE_TYPE& texture_mask floor_unused) {
}

void a2emodel::post_draw_material(gl_shader& shd floor_unused) {
}

const string a2emodel::select_shader(const DRAW_MODE& draw_mode floor_unused) const {
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
void a2emodel::set_position(const float x, const float y, const float z) {
	position.set(x, y, z);
	
	bbox.pos.set(x, y, z);
	for(unsigned int i = 0; i < object_count; i++) {
		sub_bboxes[i].pos.set(x, y, z);
	}
}

/*! sets the position of the model
 *  @param pos the new position
 */
void a2emodel::set_position(const float3& pos) {
	set_position(pos.x, pos.y, pos.z);
}

/*! sets the rotation of the model
 *  @param x the x rotation
 *  @param y the y rotation
 *  @param z the z rotation
 */
void a2emodel::set_rotation(const float x, const float y, const float z) {
	rot_mat = matrix4f().rotate_x(x) * matrix4f().rotate_y(y) * matrix4f().rotate_z(z);
	rot_mat.invert();
	
	// update bounding boxes mview matrix
	bbox.mview = rot_mat;
	for(unsigned int i = 0; i < object_count; i++) {
		sub_bboxes[i].mview = rot_mat;
	}
}

/*! sets the rotation of the model
 *  @param pos the new rotation
 */
void a2emodel::set_rotation(const float3& rot) {
	set_rotation(rot.x, rot.y, rot.z);
}

/*! sets the render scale of the model (the rendered model is scaled)
 *  @param x the x scale
 *  @param y the y scale
 *  @param z the z scale
 */
void a2emodel::set_scale(const float x, const float y, const float z) {
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
void a2emodel::set_scale(const float3& scl) {
	set_scale(scl.x, scl.y, scl.z);
}

/*! sets the "vertex scale" of the model (the model itself is scaled)
 *  @param hscl the new hard scale
 */
void a2emodel::set_hard_scale(const float3& hscl) {
	set_hard_scale(hscl.x, hscl.y, hscl.z);
}

/*! sets the "vertex positions" of the model (the model itself is relocated)
 *  @param hpos the new hard position
 */
void a2emodel::set_hard_position(const float3& hpos) {
	set_hard_scale(hpos.x, hpos.y, hpos.z);
}

/*! returns the position of the model
 */
float3& a2emodel::get_position() {
	return position;
}
const float3& a2emodel::get_position() const {
	return position;
}

/*! returns the scale of the model
 */
float3& a2emodel::get_scale() {
	return scale;
}
const float3& a2emodel::get_scale() const {
	return scale;
}

/*! returns the rotation of the model
 */
matrix4f& a2emodel::get_rotation_matrix() {
	return rot_mat;
}
const matrix4f& a2emodel::get_rotation_matrix() const {
	return rot_mat;
}

void a2emodel::set_rotation_matrix(const matrix4f& mat) {
	rot_mat = mat;
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
			smin.min(vert);
			smax.max(vert);
		}
		
		min.min(smin);
		max.max(smax);
		
		sbbox.min = smin;
		sbbox.max = smax;
		sbbox.min *= scale;
		sbbox.max *= scale;
		sbbox.mview = rot_mat;
		sbbox.pos.set(position);
	}
	
	bbox.min = min;
	bbox.max = max;
	
	// scale bbox
	bbox.min *= scale;
	bbox.max *= scale;
	
	// rotate bbox
	bbox.mview = rot_mat;
	
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
	if(sub_bboxes.empty() || sub_object >= object_count) return nullptr;
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

a2ematerial* a2emodel::get_material() const {
	return material;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// model data functions                                                                                    ////

/*! returns a pointer to the vertices
 */
float3** a2emodel::get_vertices() const {
	return model_vertices;
}

/*! returns a pointer to the vertices
 */
const float3* a2emodel::get_vertices(unsigned int obj_num) const {
	return model_vertices[obj_num];
}

/*! returns a pointer to the tex coords
 */
float2** a2emodel::get_tex_coords() const {
	return model_tex_coords;
}

/*! returns a pointer to the tex coords
 */
const float2* a2emodel::get_tex_coords(unsigned int obj_num) const {
	return model_tex_coords[obj_num];
}

/*! returns a pointer to all the indices
 */
uint3** a2emodel::get_indices() const {
	return model_indices;
}

/*! returns a pointer to the specified (by obj_num) indices
 *  @param obj_num sub-object number we want the indices from
 */
const uint3* a2emodel::get_indices(unsigned int obj_num) const {
	return model_indices[obj_num];
}

/*! returns the vertex count
 */
unsigned int a2emodel::get_vertex_count() const {
	unsigned int model_total_vertex_count = 0;
	for(unsigned int i = 0; i < object_count; i++) {
		model_total_vertex_count += model_vertex_count[i];
	}
	return model_total_vertex_count;
}

/*! returns the vertex count of the specified sub-object
 */
unsigned int a2emodel::get_vertex_count(unsigned int obj_num) const {
	return model_vertex_count[obj_num];
}

/*! returns the index count
 */
unsigned int a2emodel::get_index_count() const {
	unsigned int model_total_index_count = 0;
	for(unsigned int i = 0; i < object_count; i++) {
		model_total_index_count += model_index_count[i];
	}
	return model_total_index_count;
}

/*! returns the index count of the sub-object obj_num
 *  @param obj_num sub-object we want to get the index count from
 */
unsigned int a2emodel::get_index_count(unsigned int obj_num) const {
	return model_index_count[obj_num];
}

/*! returns a string pointer to the models sub-object names
 */
const vector<string>& a2emodel::get_object_names() const {
	return a2emodel::object_names;
}

/*! returns the models sub-object count
 */
unsigned int a2emodel::get_object_count() const {
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
uint3* a2emodel::get_col_indices() {
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

void a2emodel::set_environment_map(const GLuint env_map_) {
	env_map = env_map_;
}

GLuint a2emodel::get_environment_map() const {
	return env_map;
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
			sce->add_alpha_object(&sub_bboxes[sub_object], sub_object, bind(&a2emodel::draw_sub_object, this,
																			placeholders::_1, placeholders::_2, placeholders::_3));
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
