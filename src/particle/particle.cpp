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
	
#include "particle.hpp"
#include "particle/particle_base.hpp"
#include "particle/particle_cl.hpp"
#include "cl/opencl.hpp"

/*! there is no function currently
 */
particle_manager::particle_manager(engine* e_) : e(e_), s(e_->get_shader()), cl(e_->get_opencl()), r(e_->get_rtt()), exts(e_->get_ext()), t(e_->get_texman()) {
	//
	if(cl->is_supported()) {
		log_debug("using OpenCL render path!");
		pm = new particle_manager_cl(e);
	}
	// no hw/sw support at all
	else {
		log_error("no hardware or software support (install an opencl gpu or cpu driver)!");
		pm = nullptr;
	}
}

/*! there is no function currently
 */
particle_manager::~particle_manager() {
	log_debug("deleting particle_manager object");

	if(pm != nullptr) delete pm;

	log_debug("particle_manager object deleted");
}

/*! draws all particle systems
 */
void particle_manager::draw(const rtt::fbo* frame_buffer) {
	pm->draw(frame_buffer);
}

void particle_manager::draw_particle_system(particle_system* ps, const rtt::fbo* frame_buffer) {
	pm->draw_particle_system(ps, frame_buffer);
}

/*! runs the particle system
 */
void particle_manager::run() {
	pm->run();
}

particle_manager_base* particle_manager::get_manager() {
	return pm;
}

void particle_manager::set_manager(particle_manager_base* pm_) {
	if(pm != nullptr) {
		delete pm;
	}
	pm = pm_;
}

particle_system* particle_manager::add_particle_system(const particle_system::EMITTER_TYPE type,
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
	return pm->add_particle_system(type, ltype, tex, spawn_rate, living_time, energy, position, position_offset, extents, direction, angle, gravity, color, size, aux_data);
}

void particle_manager::delete_particle_system(particle_system* ps) {
	pm->delete_particle_system(ps);
}
