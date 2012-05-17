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

#ifndef __A2E_PARTICLE_SYSTEM_H__
#define __A2E_PARTICLE_SYSTEM_H__

#include "global.h"

#include "core/core.h"
#include "engine.h"
#include "core/bbox.h"
#include "rendering/gfx2d.h"
#include "cl/opencl.h"

/*! @class particle_system class
 *  @brief a2e particle system class
 */

#define A2E_MAX_PARTICLE_LIGHTS 4ULL

class light;
class A2E_API particle_system {
public:
	particle_system(engine* e);
	~particle_system();
	
	enum class EMITTER_TYPE : unsigned int {
		BOX,
		SPHERE,
		POINT
	};
	
	enum class LIGHTING_TYPE : unsigned int {
		NONE,
		POINT,
		POINT_PP
	};
	
	void set_type(particle_system::EMITTER_TYPE type);
	const particle_system::EMITTER_TYPE& get_type() const;
	
	void set_lighting_type(particle_system::LIGHTING_TYPE type);
	const particle_system::LIGHTING_TYPE& get_lighting_type() const;
	
	void set_spawn_rate(const unsigned long long int& spawn_rate);
	const unsigned long long int& get_spawn_rate() const;
	
	void set_living_time(const unsigned long long int& living_time);
	const unsigned long long int& get_living_time() const;
	
	void set_energy(const float& energy);
	const float& get_energy() const;
	
	void set_texture(a2e_texture& tex);
	const a2e_texture& get_texture() const;
	
	void set_position(const float3& position);
	const float3& get_position() const;
	
	void set_position_offset(const float3& position_offset);
	const float3& get_position_offset() const;
	
	void set_extents(const float3& extents);
	const float3& get_extents() const;
	
	void set_direction(const float3& direction);
	const float3& get_direction() const;
	
	void set_angle(const float3& angle);
	const float3& get_angle() const;
	
	void set_gravity(const float3& gravity);
	const float3& get_gravity() const;
	
	void set_color(const float4& color);
	const float4& get_color() const;
	
	void set_size(const float& size_x, const float& size_y);
	void set_size(const float2& size);
	const float2& get_size() const;
	
	void set_blend_mode(const gfx2d::BLEND_MODE mode);
	const gfx2d::BLEND_MODE& get_blend_mode() const;
	
	void set_visible(const bool state);
	bool is_visible() const;
	
	void set_active(const bool state);
	bool is_active() const;
	
	void set_sorting(const bool state);
	bool is_sorting() const;
	
	//! sorting is distributed over multiple frames/steps (~step_size global items per step)
	void set_reentrant_sorting(const bool state, const size_t& step_size);
	size_t get_reentrant_sorting_size() const;
	bool is_reentrant_sorting() const;
	
	//! whether to render the buffer that is currently being sorted (default: false)
	//! only applies to reentrant sorting!
	void set_render_intermediate_sorted_buffer(const bool state);
	bool is_render_intermediate_sorted_buffer() const;
	
	void set_lights(const vector<light*>& lights);
	const vector<light*> get_lights() const;
	const GLuint get_lights_ubo() const;
	
	const extbbox& get_bounding_box() const;
	
	void set_aux_data(void* aux_data);
	void* get_aux_data() const;
	
	//! don't set/get these from the outside!	
	struct internal_particle_data {
		unsigned long long int particle_count = 0;  // will get computed
		unsigned long long int spawn_rate_ts = 0;   // will get computed
		unsigned long long int max_init_time = 0;   // will get computed
		
		//
		size_t step_timer = 0;
		size_t reinit_timer = 0;
		GLuint particle_indices_vbo[2] = { 0, 0 };
		unsigned int particle_indices_swap = 0; // either 0 or 1
		
#if !defined(A2E_NO_OPENCL)
		// for opencl computed particle systems
		opencl::buffer_object* ocl_pos_time_buffer = nullptr;
		opencl::buffer_object* ocl_dir_buffer = nullptr;
		opencl::buffer_object* ocl_indices[2] = { nullptr, nullptr };
		opencl::buffer_object* ocl_distances = nullptr;
		GLuint ocl_gl_pos_time_vbo = 0;
		GLuint ocl_gl_dir_vbo = 0;
		cl::NDRange ocl_range_global;
#endif
		
		// vars for reentrant sorting
		bool reentrant_complete = false;
		unsigned int reentrant_cur_size = 0;
		unsigned int reentrant_cur_stride = 0;
	};
	internal_particle_data* get_internal_particle_data();
		
protected:
	engine* e;
	EMITTER_TYPE type;
	LIGHTING_TYPE lighting_type;
	unsigned long long int spawn_rate;	// should be a multiple of 25
	unsigned long long int living_time;	// should be a multiple of 40
	float energy;
	a2e_texture tex;
	float3 position;
	float3 position_offset;
	float3 extents;
	float3 direction;
	float3 angle;
	float3 gravity;
	float4 color;
	float2 size;
	bool visible;
	bool active;
	bool sorting;
	bool rentrant_sorting;
	bool render_intermediate_sorted_buffer;
	size_t sorting_step_size;
	gfx2d::BLEND_MODE blend_mode;
	vector<light*> lights;
	GLuint lights_ubo;
	void* aux_data;
	
	extbbox bbox;
	
	internal_particle_data data;
	
};

#endif
