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

#include "particle_system.h"

/*! there is no function currently
 */
particle_system::particle_system(engine* e_) : e(e_) {
	visible = true;
	active = true;
	sorting = false;
	rentrant_sorting = false;
	render_intermediate_sorted_buffer = false;
	sorting_step_size = 0;
	
	type = EMITTER_TYPE::BOX;
	lighting_type = LIGHTING_TYPE::NONE;
	
	aux_data = nullptr;
	
#if !defined(A2E_IOS)
	// only gen if ltype == POINT is set?
	glGenBuffers(1, &lights_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
	glBufferData(GL_UNIFORM_BUFFER,
				 (sizeof(float4) * 2) * A2E_MAX_PARTICLE_LIGHTS,
				 nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
#else
	lights_ubo = 0;
#endif
	
	bbox.min.set(0.0f, 0.0f, 0.0f);
	bbox.max.set(0.0f, 0.0f, 0.0f);
	bbox.pos.set(0.0f, 0.0f, 0.0f);
	
	blend_mode = gfx2d::BLEND_MODE::ADD;
	
	// init data	
#if !defined(A2E_NO_OPENCL)
	// for opencl computed particle systems
	data.ocl_range_global.set(0);
#endif
}

/*! there is no function currently
 */
particle_system::~particle_system() {
	if(glIsBuffer(lights_ubo)) glDeleteBuffers(1, &lights_ubo);
	

#if !defined(A2E_NO_OPENCL)
	if(data.ocl_pos_time_buffer != nullptr) e->get_opencl()->delete_buffer(data.ocl_pos_time_buffer);
	if(data.ocl_dir_buffer != nullptr) e->get_opencl()->delete_buffer(data.ocl_dir_buffer);
	if(data.ocl_distances != nullptr) e->get_opencl()->delete_buffer(data.ocl_distances);
	if(data.ocl_indices[0] != nullptr) e->get_opencl()->delete_buffer(data.ocl_indices[0]);
	if(data.ocl_indices[1] != nullptr) e->get_opencl()->delete_buffer(data.ocl_indices[1]);
	if(glIsBuffer(data.ocl_gl_pos_time_vbo)) glDeleteBuffers(1, &data.ocl_gl_pos_time_vbo);
	if(glIsBuffer(data.ocl_gl_dir_vbo)) glDeleteBuffers(1, &data.ocl_gl_dir_vbo);
#endif
	
	if(glIsBuffer(data.particle_indices_vbo[0])) glDeleteBuffers(1, &data.particle_indices_vbo[0]);
	if(glIsBuffer(data.particle_indices_vbo[1])) glDeleteBuffers(1, &data.particle_indices_vbo[1]);
}

void particle_system::set_type(particle_system::EMITTER_TYPE type_) {
	type = type_;
}

const particle_system::EMITTER_TYPE& particle_system::get_type() const {
	return type;
}

void particle_system::set_lighting_type(particle_system::LIGHTING_TYPE type_) {
	lighting_type = type_;
}

const particle_system::LIGHTING_TYPE& particle_system::get_lighting_type() const {
	return lighting_type;
}

void particle_system::set_spawn_rate(const unsigned long long int& spawn_rate_) {
	spawn_rate = spawn_rate_;
}

const unsigned long long int& particle_system::get_spawn_rate() const {
	return spawn_rate;
}

void particle_system::set_living_time(const unsigned long long int& living_time_) {
	living_time = living_time_;
}

const unsigned long long int& particle_system::get_living_time() const {
	return living_time;
}

void particle_system::set_energy(const float& energy_) {
	energy = energy_;
}

const float& particle_system::get_energy() const {
	return energy;
}

void particle_system::set_texture(a2e_texture& tex_) {
	tex = tex_;
}

const a2e_texture& particle_system::get_texture() const {
	return tex;
}

void particle_system::set_position(const float3& position_) {
	position.set(position_);
	bbox.pos.set(position);
}

const float3& particle_system::get_position() const {
	return position;
}

void particle_system::set_position_offset(const float3& position_offset_) {
	position_offset.set(position_offset_);
}

const float3& particle_system::get_position_offset() const {
	return position_offset;
}

void particle_system::set_extents(const float3& extents_) {
	extents.set(extents_);
	bbox.min.set(extents);
	bbox.max.set(extents);
	bbox.min *= -0.5f;
	bbox.max *= 0.5f;
}

const float3& particle_system::get_extents() const {
	return extents;
}

void particle_system::set_direction(const float3& direction_) {
	direction.set(direction_);
	direction.normalize();
	
	// check for zero length (0, 0, 0), if so, set direction y to 1
	if(direction.length() == 0.0f) {
		direction.y = 1.0f;
	}
}

const float3& particle_system::get_direction() const {
	return direction;
}

void particle_system::set_angle(const float3& angle_) {
	angle.set(angle_);
}

const float3& particle_system::get_angle() const {
	return angle;
}

void particle_system::set_gravity(const float3& gravity_) {
	gravity.set(gravity_);
}

const float3& particle_system::get_gravity() const {
	return gravity;
}

void particle_system::set_color(const float4& color_) {
	particle_system::color.set(color_);
}

const float4& particle_system::get_color() const {
	return color;
}

void particle_system::set_size(const float& size_x, const float& size_y) {
	size.set(size_x, size_y);
}

void particle_system::set_size(const float2& size_) {
	size = size_;
}

const float2& particle_system::get_size() const {
	return size;
}

particle_system::internal_particle_data* particle_system::get_internal_particle_data() {
	return &data;
}

const extbbox& particle_system::get_bounding_box() const {
	return bbox;
}

void particle_system::set_visible(const bool state) {
	visible = state;
}

bool particle_system::is_visible() const {
	return visible;
}

void particle_system::set_blend_mode(const gfx2d::BLEND_MODE mode) {
	blend_mode = mode;
}

const gfx2d::BLEND_MODE& particle_system::get_blend_mode() const {
	return blend_mode;
}

void particle_system::set_lights(const vector<light*>& lights_) {
	lights = lights_;
}

const vector<light*> particle_system::get_lights() const {
	return lights;
}

const GLuint& particle_system::get_lights_ubo() const {
	return lights_ubo;
}

void particle_system::set_active(const bool state) {
	active = state;
}

bool particle_system::is_active() const {
	return active;
}

void particle_system::set_aux_data(void* aux_data_) {
	aux_data = aux_data_;
}

void* particle_system::get_aux_data() const {
	return aux_data;
}

void particle_system::set_sorting(const bool state) {
	sorting = state;
	set_blend_mode(sorting ? gfx2d::BLEND_MODE::PRE_MUL : gfx2d::BLEND_MODE::ADD);
}

bool particle_system::is_sorting() const {
	return sorting;
}

void particle_system::set_reentrant_sorting(const bool state, const size_t& step_size) {
	rentrant_sorting = state;
	sorting_step_size = step_size;
	if(rentrant_sorting && !sorting) {
		set_sorting(true);
	}
}

size_t particle_system::get_reentrant_sorting_size() const {
	return sorting_step_size;
}

bool particle_system::is_reentrant_sorting() const {
	return rentrant_sorting;
}

void particle_system::set_render_intermediate_sorted_buffer(const bool state) {
	render_intermediate_sorted_buffer = state;
}

bool particle_system::is_render_intermediate_sorted_buffer() const {
	return render_intermediate_sorted_buffer;
}
