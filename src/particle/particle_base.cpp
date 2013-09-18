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

#include "particle_base.hpp"
#include "cl/opencl.hpp"

particle_manager_base::particle_manager_base() :
s(engine::get_shader()), r(engine::get_rtt()), exts(engine::get_ext()), t(engine::get_texman()) {
	max_particle_count = 1024*128; // limit to 2^17 for now
}

particle_manager_base::~particle_manager_base() {
	log_debug("deleting particle_manager_base object");
	
	for(const auto& psystem : particle_systems) {
		delete psystem;
	}
	particle_systems.clear();
	
	log_debug("particle_manager_base object deleted");
}

/*! draws all particle systems
 */
void particle_manager_base::draw(const rtt::fbo* frame_buffer) {
	for(const auto& psystem : particle_systems) {
		if(psystem->is_visible()) draw_particle_system(psystem, frame_buffer);
	}
}

/*! runs the particle system
 */
void particle_manager_base::run() {
	for(const auto& psystem : particle_systems) {
		if(psystem->is_active()) run_particle_system(psystem);
	}
}

/*! deletes the specified particle system
 */
void particle_manager_base::delete_particle_system(particle_system* ps) {
	particle_systems.erase(ps);
	delete ps;
}

particle_system* particle_manager_base::init(const particle_system::EMITTER_TYPE type,
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
											 const float2 size,
											 void* aux_data) {
	particle_system* ps = new particle_system();
	ps->set_type(type);
	ps->set_lighting_type(ltype);
	ps->set_spawn_rate(spawn_rate);
	ps->set_living_time(living_time);
	ps->set_energy(energy);
	ps->set_texture(tex);
	ps->set_angle(angle);
	ps->set_position(position);
	ps->set_position_offset(position_offset);
	ps->set_extents(extents);
	ps->set_direction(direction);
	ps->set_gravity(gravity);
	ps->set_color(color);
	ps->set_size(size);
	ps->set_aux_data(aux_data);
	particle_systems.insert(ps);
	return ps;
}

void particle_manager_base::compute_particle_count(particle_system* ps) {
	particle_system::internal_particle_data* pdata = ps->get_internal_particle_data();
	
	pdata->particle_count = (ps->get_living_time() * ps->get_spawn_rate()) / 1000;
	if(pdata->particle_count > max_particle_count) {
		log_debug("particle_count higher than allowed count of %u! restricting to this size now!", max_particle_count);
		ps->set_spawn_rate((unsigned long long int)floorf((float)max_particle_count * (1000.0f / (float)ps->get_living_time())));
		pdata->particle_count = (ps->get_living_time() * ps->get_spawn_rate()) / 1000;
	}
	
	pdata->spawn_rate_ts = ps->get_spawn_rate() / 25;
	pdata->max_init_time = ((unsigned long long int)((float)pdata->particle_count / (float)pdata->spawn_rate_ts) - 1) * 40;
}
