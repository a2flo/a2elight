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

#include "particle_cl.h"
#include "scene/light.h"

#if !defined(A2E_NO_OPENCL)

/*! there is no function currently
 */
particle_manager_cl::particle_manager_cl(engine* e) : particle_manager_base(e) {
	//max_particle_count = 1024*1024; // limit to 1 million+
	max_particle_count = 16*1024*1024; // gna, screw it, 16 mil should be fine
	
	cl->use_kernel("PARTICLE INIT");
	init_range_local = cl->compute_local_kernel_range(1);
	cl->use_kernel("PARTICLE RESPAWN");
	respawn_range_local = cl->compute_local_kernel_range(1);
	cl->use_kernel("PARTICLE COMPUTE");
	compute_range_local = cl->compute_local_kernel_range(1);
	cl->use_kernel("PARTICLE SORT MERGE GLOBAL");
	sort_range_local = cl->compute_local_kernel_range(1);
	cl->use_kernel("PARTICLE COMPUTE DISTANCES");
	compute_distances_local = cl->compute_local_kernel_range(1);
	
	// find least common multiple of all local kernel sizes (will then be used for making sure that particle count is always a multiple of this)
	local_lcm = core::lcm(core::lcm(init_range_local[0], respawn_range_local[0]), compute_range_local[0]);
	cl->use_kernel("PARTICLE INIT");
	
	kernel_seed = (unsigned int)time(NULL);
}

/*! there is no function currently
 */
particle_manager_cl::~particle_manager_cl() {
	a2e_debug("deleting particle_manager_cl object");
	
	a2e_debug("particle_manager_cl object deleted");
}

/*! adds a new particle system (position, color and direction should be created within the function call - they will get deleted in this function)
 */
particle_system* particle_manager_cl::add_particle_system(const particle_system::EMITTER_TYPE type,
														  const particle_system::LIGHTING_TYPE ltype,
														  a2e_texture& tex,
														  const unsigned long long int spawn_rate,
														  const unsigned long long int living_time,
														  const float energy,
														  const float3 position,
														  const float3 position_offset,
														  const float3 extents,
														  const float3 direction,
														  const float3 angle,
														  const float3 gravity,
														  const float4 color,
														  const float2 size) {
	particle_system* ps = init(type, ltype, tex, spawn_rate, living_time, energy, position, position_offset, extents, direction, angle, gravity, color, size);
	
	reset_particle_system(ps);
	
	return ps;
}

void particle_manager_cl::compute_particle_count(particle_system* ps) {
	particle_system::internal_particle_data* pdata = ps->get_internal_particle_data();
	particle_manager_base::compute_particle_count(ps); // call parent
	
	// make sure particle count is dividable by all kernel local sizes
	/*if((pdata->particle_count % local_lcm) != 0) {
		const unsigned long long int living_time = ps->get_living_time();
		
		// compute appropriate particle count, also: min particle count = local_lcm
		pdata->particle_count = (unsigned long long int)std::max((((unsigned long long int)pdata->particle_count / local_lcm) * local_lcm), local_lcm);
		
		const unsigned long long int spawn_rate = (pdata->particle_count * 1000) / living_time;
		ps->set_spawn_rate(spawn_rate);
		
		pdata->spawn_rate_ts = ps->get_spawn_rate() / 25;
		pdata->max_init_time = ((unsigned long long int)((float)pdata->particle_count / (float)pdata->spawn_rate_ts) - 1) * 40;
	}*/
	
	// particle count must be a power of two
	pdata->particle_count = core::next_pot(pdata->particle_count);
}

void particle_manager_cl::reset_particle_count(particle_system* ps) {
	particle_system::internal_particle_data* pdata = ps->get_internal_particle_data();
	
	// delete old data, if there is some ...
	if(glIsBuffer(pdata->ocl_gl_pos_time_vbo)) glDeleteBuffers(1, &pdata->ocl_gl_pos_time_vbo);
	if(glIsBuffer(pdata->ocl_gl_dir_vbo)) glDeleteBuffers(1, &pdata->ocl_gl_dir_vbo);
	if(glIsBuffer(pdata->particle_indices_vbo[0])) glDeleteBuffers(1, &pdata->particle_indices_vbo[0]);
	if(glIsBuffer(pdata->particle_indices_vbo[1])) glDeleteBuffers(1, &pdata->particle_indices_vbo[1]);
	if(pdata->ocl_pos_time_buffer != NULL) cl->delete_buffer(pdata->ocl_pos_time_buffer);
	if(pdata->ocl_dir_buffer != NULL) cl->delete_buffer(pdata->ocl_dir_buffer);
	if(pdata->ocl_distances != NULL) cl->delete_buffer(pdata->ocl_distances);
	
	// compute new particle count
	compute_particle_count(ps);
	
	// init/allocate data
	float4* pos_time_data = new float4[pdata->particle_count];
	float4* dir_data = new float4[pdata->particle_count];
	
	for(unsigned int i = 0; i < pdata->particle_count; i++) {
		pos_time_data[i].x = 0.0f;
		pos_time_data[i].y = 0.0f;
		pos_time_data[i].z = 0.0f;
		pos_time_data[i].w = 0.0f;
		
		dir_data[i].x = 0.0f;
		dir_data[i].y = 0.0f;
		dir_data[i].z = 0.0f;
		dir_data[i].w = 0.0f;
	}
	
	glGenBuffers(1, &pdata->ocl_gl_pos_time_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pdata->ocl_gl_pos_time_vbo);
	glBufferData(GL_ARRAY_BUFFER, pdata->particle_count * sizeof(float4), pos_time_data, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &pdata->ocl_gl_dir_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pdata->ocl_gl_dir_vbo);
	glBufferData(GL_ARRAY_BUFFER, pdata->particle_count * sizeof(float4), dir_data, GL_DYNAMIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	delete [] pos_time_data;
	delete [] dir_data;
	
	pdata->ocl_pos_time_buffer = cl->create_ogl_buffer(opencl::BT_READ_WRITE, pdata->ocl_gl_pos_time_vbo);
	pdata->ocl_dir_buffer = cl->create_ogl_buffer(opencl::BT_READ_WRITE, pdata->ocl_gl_dir_vbo);
	pdata->ocl_range_global = pdata->particle_count;
	
	cl->set_manual_gl_sharing(pdata->ocl_pos_time_buffer, true);
	cl->set_manual_gl_sharing(pdata->ocl_dir_buffer, true);
	
	// create/init particle indices
	unsigned int* particle_indices = new unsigned int[pdata->particle_count];
	for(unsigned int i = 0; i < pdata->particle_count; i++) {
		particle_indices[i] = i;
	}
	
	for(unsigned int i = 0; i < 2; i++) {
		glGenBuffers(1, &pdata->particle_indices_vbo[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pdata->particle_indices_vbo[i]);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, pdata->particle_count * sizeof(unsigned int), particle_indices, GL_STATIC_READ);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pdata->particle_count * sizeof(unsigned int), particle_indices, GL_DYNAMIC_DRAW);
		
		pdata->ocl_indices[i] = cl->create_ogl_buffer(opencl::BT_READ_WRITE, pdata->particle_indices_vbo[i]);
		cl->set_manual_gl_sharing(pdata->ocl_indices[i], true);
	}
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	delete [] particle_indices;
	
	// create distances buffer for sorting
	pdata->ocl_distances = cl->create_buffer(opencl::BT_READ_WRITE, pdata->particle_count * sizeof(float), NULL);
	
	a2e_debug("particle count: %u", ps->get_internal_particle_data()->particle_count);
}

void particle_manager_cl::reset_particle_system(particle_system* ps) {
	particle_system::internal_particle_data* pdata = ps->get_internal_particle_data();
	
	if(pdata->particle_count != (ps->get_living_time() * ps->get_spawn_rate()) / 1000) {
		reset_particle_count(ps);
	}
	
	cl->acquire_gl_object(pdata->ocl_pos_time_buffer);
	cl->acquire_gl_object(pdata->ocl_dir_buffer);
	
	cl->use_kernel("PARTICLE INIT");
	cl->set_kernel_argument(0, (unsigned int)ps->get_type());
	cl->set_kernel_argument(1, (float)ps->get_living_time());
	cl->set_kernel_argument(2, (unsigned int)pdata->particle_count);
	cl->set_kernel_argument(3, (float)ps->get_energy());
	cl->set_kernel_argument(4, (float4)ps->get_angle());
	cl->set_kernel_argument(5, (float4)ps->get_extents());
	cl->set_kernel_argument(6, (float4)ps->get_direction());
	cl->set_kernel_argument(7, (float4)ps->get_position_offset());
	cl->set_kernel_argument(8, kernel_seed);
	cl->set_kernel_argument(9, (float)pdata->spawn_rate_ts);
	cl->set_kernel_argument(10, pdata->ocl_pos_time_buffer);
	cl->set_kernel_argument(11, pdata->ocl_dir_buffer);
	//cl->set_kernel_range(pdata->ocl_range_global, init_range_local);
	cl->set_kernel_range(pdata->ocl_range_global, cl::NDRange(256)); // TODO: compute local ws size
	cl->run_kernel();
	
	cl->release_gl_object(pdata->ocl_pos_time_buffer);
	cl->release_gl_object(pdata->ocl_dir_buffer);
	
	pdata->step_timer = SDL_GetTicks() - 1000;
	pdata->reinit_timer = SDL_GetTicks();
}

void particle_manager_cl::run_particle_system(particle_system* ps) {
	particle_system::internal_particle_data* pdata = ps->get_internal_particle_data();
	bool updated = false;
	
	// delete old particles and create new ones
	if(SDL_GetTicks() - pdata->reinit_timer > 40) { // do this 25 times every second (1000 / 25 = 40)
		// update seed
		kernel_seed = (unsigned int)time(NULL);
		
		cl->acquire_gl_object(pdata->ocl_pos_time_buffer);
		cl->acquire_gl_object(pdata->ocl_dir_buffer);
		
		cl->use_kernel("PARTICLE RESPAWN");
		cl->set_kernel_argument(0, ps->get_type());
		cl->set_kernel_argument(1, (float)ps->get_living_time());
		cl->set_kernel_argument(2, (unsigned int)pdata->particle_count);
		cl->set_kernel_argument(3, ps->get_energy());
		cl->set_kernel_argument(4, (float4)ps->get_angle());
		cl->set_kernel_argument(5, (float4)ps->get_extents());
		cl->set_kernel_argument(6, (float4)ps->get_direction());
		cl->set_kernel_argument(7, (float4)ps->get_position_offset());
		cl->set_kernel_argument(8, kernel_seed);
		cl->set_kernel_argument(9, (float4)ps->get_gravity());
		cl->set_kernel_argument(10, pdata->ocl_pos_time_buffer);
		cl->set_kernel_argument(11, pdata->ocl_dir_buffer);
		//cl->set_kernel_range(pdata->ocl_range_global, respawn_range_local);
		cl->set_kernel_range(pdata->ocl_range_global, cl::NDRange(256)); // TODO: compute local ws size
		cl->run_kernel();
		
		pdata->reinit_timer = SDL_GetTicks();
		updated = true;
	}
	if(SDL_GetTicks() - pdata->step_timer > 10) {
		if(!updated) {
			// only acquire gl objects if the respawn kernel hasn't acquired them already
			cl->acquire_gl_object(pdata->ocl_pos_time_buffer);
			cl->acquire_gl_object(pdata->ocl_dir_buffer);
		}
		
		// update positions
		cl->use_kernel("PARTICLE COMPUTE");
		cl->set_kernel_argument(0, (float)(SDL_GetTicks() - pdata->step_timer));
		cl->set_kernel_argument(1, (float)ps->get_living_time());
		cl->set_kernel_argument(2, (unsigned int)pdata->particle_count);
		cl->set_kernel_argument(3, (float4)ps->get_gravity());
		cl->set_kernel_argument(4, pdata->ocl_pos_time_buffer);
		cl->set_kernel_argument(5, pdata->ocl_dir_buffer);
		cl->set_kernel_range(pdata->ocl_range_global, compute_range_local);
		cl->run_kernel();

		pdata->step_timer = SDL_GetTicks();
		updated = true;
	}
	
	// don't wait for another update when using reentrant sorting (and it's not complete yet)
	if(ps->is_reentrant_sorting() && !pdata->reentrant_complete) {
		sort_particle_system(ps);
	}
	
	if(updated) {
		// only start sorting when there is an update
		if(ps->is_sorting() &&
		   // sort here when not using reentrant sorting or when reentrant sorting has completed
		   (!ps->is_reentrant_sorting() || (ps->is_reentrant_sorting() && pdata->reentrant_complete))) {
			sort_particle_system(ps);
		}
		
		// release everything that has been acquired before
		cl->release_gl_object(pdata->ocl_pos_time_buffer);
		cl->release_gl_object(pdata->ocl_dir_buffer);
	}
}

void particle_manager_cl::sort_particle_system(particle_system* ps) {
	particle_system::internal_particle_data* pdata = ps->get_internal_particle_data();
	
	// nvidia bitonic merge sort:
	static bool debug_lsize = false;
	if(!debug_lsize) {
		debug_lsize = true;
		cl->use_kernel("PARTICLE SORT LOCAL");
		size_t wgs = cl->get_kernel_work_group_size();
		a2e_debug("PARTICLE SORT LOCAL wgs: %u", wgs);
		cl->use_kernel("PARTICLE SORT MERGE GLOBAL");
		wgs = cl->get_kernel_work_group_size();
		a2e_debug("PARTICLE SORT MERGE GLOBAL wgs: %u", wgs);
		cl->use_kernel("PARTICLE SORT MERGE LOCAL");
		wgs = cl->get_kernel_work_group_size();
		a2e_debug("PARTICLE SORT MERGE LOCAL wgs: %u", wgs);
		a2e_debug("PARTICLE SORT local_size_limit: %u", cl->get_active_device()->max_wg_size);
	}
	
	const unsigned int local_size_limit = (unsigned int)cl->get_active_device()->max_wg_size; // TODO: actually use the compiled/build value
	const bool reentrant_sorting = ps->is_reentrant_sorting();
	const size_t reentrant_sorting_size = ps->get_reentrant_sorting_size();
	unsigned int arg_num = 0;
	size_t overall_global_size = 0;
	
	static size_t reentry_counter = 0;
	if(!reentrant_sorting || pdata->reentrant_complete) {
		reentry_counter = 0;
		// note: at this point, we have already acquired ocl_pos_time_buffer and ocl_dir_buffer,
		// so only both indicies buffers must be acquired
		cl->acquire_gl_object(pdata->ocl_indices[0]);
		cl->acquire_gl_object(pdata->ocl_indices[1]);
		
		//
		float4 camera_pos(-*e->get_position(), 1.0f);
		pdata->particle_indices_swap = 1 - pdata->particle_indices_swap; // swap
		
		// compute particle distances
		cl->use_kernel("PARTICLE COMPUTE DISTANCES");
		cl->set_kernel_argument(arg_num++, pdata->ocl_pos_time_buffer);
		cl->set_kernel_argument(arg_num++, camera_pos);
		cl->set_kernel_argument(arg_num++, pdata->ocl_distances);
		cl::NDRange compute_distances_global(pdata->particle_count);
		cl->set_kernel_range(compute_distances_global, compute_distances_local);
		cl->run_kernel();
		
		// first sorting step
		arg_num = 0;
		cl->use_kernel("PARTICLE SORT LOCAL");
		cl->set_kernel_argument(arg_num++, pdata->ocl_distances);
		cl->set_kernel_argument(arg_num++, pdata->ocl_indices[pdata->particle_indices_swap]);
		cl->set_kernel_argument(arg_num++, pdata->ocl_indices[1 - pdata->particle_indices_swap]);
		cl::NDRange sort_local1_local(local_size_limit / 2);
		cl::NDRange sort_local1_global(pdata->particle_count / 2);
		cl->set_kernel_range(sort_local1_global, sort_local1_local);
		cl->run_kernel();
		
		// this is not needed any more, release it
		cl->release_gl_object(pdata->ocl_indices[1 - pdata->particle_indices_swap]);
		
		// set loop vars
		pdata->reentrant_complete = false;
		pdata->reentrant_cur_size = 2 * local_size_limit;
		pdata->reentrant_cur_stride = 0;
		
		//
		overall_global_size += sort_local1_global[0];
		if(reentrant_sorting &&
		   overall_global_size > reentrant_sorting_size) {
			if(ps->is_render_intermediate_sorted_buffer()) {
				cl->release_gl_object(pdata->ocl_indices[pdata->particle_indices_swap]);
			}
			return;
		}
	}
	else {
		reentry_counter++;
		// new sorting step, acquire indices buffer again
		if(ps->is_render_intermediate_sorted_buffer()) { // this is still acquired when we're not rendering the buffer
			cl->acquire_gl_object(pdata->ocl_indices[pdata->particle_indices_swap]);
		}
	}
	
	for(unsigned int size = pdata->reentrant_cur_size; size <= pdata->particle_count; size <<= 1) {
		pdata->reentrant_cur_size = size;
		if(pdata->reentrant_cur_stride == 0) {
			pdata->reentrant_cur_stride = size / 2;
		}
		
		for(unsigned int stride = pdata->reentrant_cur_stride; stride > 0; stride >>= 1) {
			//
			pdata->reentrant_cur_stride = stride;
			if(reentrant_sorting &&
			   overall_global_size > reentrant_sorting_size) {
				if(ps->is_render_intermediate_sorted_buffer()) {
					cl->release_gl_object(pdata->ocl_indices[pdata->particle_indices_swap]);
				}
				return;
			}
			
			if(stride >= local_size_limit) {
				cl->use_kernel("PARTICLE SORT MERGE GLOBAL");
				arg_num = 0;
				cl->set_kernel_argument(arg_num++, pdata->ocl_distances);
				cl->set_kernel_argument(arg_num++, pdata->ocl_indices[pdata->particle_indices_swap]);
				cl->set_kernel_argument(arg_num++, pdata->ocl_indices[pdata->particle_indices_swap]);
				cl->set_kernel_argument(arg_num++, (unsigned int)pdata->particle_count);
				cl->set_kernel_argument(arg_num++, size);
				cl->set_kernel_argument(arg_num++, stride);
				
				cl::NDRange merge_global_global(pdata->particle_count / 2);
				//cl::NDRange merge_global_local(256); // TODO: compute this
				cl::NDRange merge_global_local(local_size_limit / 2);
				cl->set_kernel_range(merge_global_global, merge_global_local);
				cl->run_kernel();
				overall_global_size += merge_global_global[0];
			}
			else {
				cl->use_kernel("PARTICLE SORT MERGE LOCAL");
				arg_num = 0;
				cl->set_kernel_argument(arg_num++, pdata->ocl_distances);
				cl->set_kernel_argument(arg_num++, pdata->ocl_indices[pdata->particle_indices_swap]);
				cl->set_kernel_argument(arg_num++, pdata->ocl_indices[pdata->particle_indices_swap]);
				cl->set_kernel_argument(arg_num++, (unsigned int)pdata->particle_count);
				cl->set_kernel_argument(arg_num++, size);
				cl->set_kernel_argument(arg_num++, stride);
				
				cl::NDRange merge_local_local(local_size_limit / 2);
				cl::NDRange merge_local_global(pdata->particle_count / 2);
				cl->set_kernel_range(merge_local_global, merge_local_local);
				cl->run_kernel();
				overall_global_size += merge_local_global[0];
				break;
			}
		}
		pdata->reentrant_cur_stride = 0;
		
		if((size << 1) > pdata->particle_count) {
			pdata->reentrant_cur_size = 0;
			pdata->reentrant_complete = true;
		}
	}
	
	// and release again
	cl->release_gl_object(pdata->ocl_indices[pdata->particle_indices_swap]);
}

void particle_manager_cl::draw_particle_system(particle_system* ps, const rtt::fbo* frame_buffer) {
	// prep matrices
	matrix4f mvm(*e->get_modelview_matrix());
	matrix4f pm(*e->get_projection_matrix());
	matrix4f mvpm(mvm * pm);
	
	// draw
	particle_system::internal_particle_data* pdata = ps->get_internal_particle_data();
	
	glEnable(GL_BLEND);
	g->set_blend_mode(ps->get_blend_mode());
	glDepthMask(GL_FALSE);
	
	// point -> gs: quad
	gl3shader particle_draw = s->get_gl3shader("PARTICLE_DRAW_OPENCL");
#if !defined(A2E_IOS)
	const auto ltype = ps->get_lighting_type();
#else
	const auto ltype = particle_system::LIGHTING_TYPE::NONE; // can't use particle lighting on iOS/GLES2.0
#endif
	string shd_option = "#";
	switch(ltype) {
		case particle_system::LIGHTING_TYPE::NONE: shd_option = "#"; break;
#if !defined(A2E_IOS)
		case particle_system::LIGHTING_TYPE::POINT: shd_option = "lit"; break;
		case particle_system::LIGHTING_TYPE::POINT_PP: shd_option = "lit_pp"; break;
#endif
	}
	particle_draw->use(shd_option);
	particle_draw->uniform("living_time", (float)ps->get_living_time());
	particle_draw->uniform("mvm", mvm);
	particle_draw->uniform("mvpm", mvpm);
	particle_draw->uniform("position", ps->get_position());
	
	const float2 near_far_plane = e->get_near_far_plane();
	const float2 projection_ab = float2(near_far_plane.y / (near_far_plane.y - near_far_plane.x),
										(-near_far_plane.y * near_far_plane.x) / (near_far_plane.y - near_far_plane.x));
	particle_draw->uniform("projection_ab", projection_ab);
	particle_draw->texture("depth_buffer", frame_buffer->depth_buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	
#if !defined(A2E_IOS)
	if(ltype != particle_system::LIGHTING_TYPE::NONE) {
		// note: max lights is currently 4
		struct __attribute__((packed, aligned(4))) light_info {
			float4 position;
			float4 color;
		} lights_data[A2E_MAX_PARTICLE_LIGHTS];
		const auto& lights = ps->get_lights();
		const size_t actual_lights = std::min(lights.size(), (size_t)A2E_MAX_PARTICLE_LIGHTS);
		for(size_t i = 0; i < actual_lights; i++) {
			lights_data[i].position = float4(lights[i]->get_position(),
											 lights[i]->get_sqr_radius());
			lights_data[i].color = float4(lights[i]->get_color(),
										  lights[i]->get_inv_sqr_radius());
		}
		for(size_t i = actual_lights; i < A2E_MAX_PARTICLE_LIGHTS; i++) {
			lights_data[i].position = float4(0.0f);
			lights_data[i].color = float4(0.0f);
		}
		// update and set ubo
		const GLuint lights_ubo = ps->get_lights_ubo();
		glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo); // will be unbound automatically
		glBufferSubData(GL_UNIFORM_BUFFER, 0,
						(sizeof(float4) * 2) * A2E_MAX_PARTICLE_LIGHTS,
						&lights_data[0]);
		particle_draw->block("light_buffer", lights_ubo);
	}
#endif
	
	particle_draw->uniform("color", ps->get_color());
	particle_draw->uniform("size", ps->get_size());
	particle_draw->texture("particle_tex", ps->get_texture());
	particle_draw->attribute_array("in_vertex", pdata->ocl_gl_pos_time_vbo, 4);
	particle_draw->attribute_array("in_aux", pdata->ocl_gl_dir_vbo, 4);
	
	if(!ps->is_sorting() ||
	   (ps->is_sorting() && !ps->is_reentrant_sorting()) ||
	   (ps->is_reentrant_sorting() && ps->is_render_intermediate_sorted_buffer())) {
		// std: use active indices buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pdata->particle_indices_vbo[pdata->particle_indices_swap]);
	}
	else if(ps->is_reentrant_sorting() && !ps->is_render_intermediate_sorted_buffer()) {
		// use previously sorted indices buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pdata->particle_indices_vbo[1 - pdata->particle_indices_swap]);
	}
	else {
		assert(false && "invalid particle system state");
	}
	
	glDrawElements(GL_POINTS, (GLsizei)pdata->particle_count, GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	particle_draw->disable();
	
	glDepthMask(GL_TRUE);
	g->set_blend_mode(gfx::BM_DEFAULT);
}

#endif
