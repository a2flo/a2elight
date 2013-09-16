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

#ifndef __A2E_PARTICLE_CL_HPP__
#define __A2E_PARTICLE_CL_HPP__

#include "global.hpp"

#include "core/core.hpp"
#include "engine.hpp"
#include "rendering/rtt.hpp"
#include "rendering/shader.hpp"
#include "rendering/texman.hpp"
#include "particle/particle_base.hpp"
#include "cl/opencl.hpp"

/*! @class particle_manager opencl class
 *  @brief a2e particle manager opencl class
 */

class A2E_API particle_manager_cl : public particle_manager_base {
public:
	particle_manager_cl(engine* e);
	virtual ~particle_manager_cl();
	
	virtual particle_system* add_particle_system(const particle_system::EMITTER_TYPE type,
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
												 void* aux_data);
	virtual void reset_particle_system(particle_system* ps);
	virtual void run_particle_system(particle_system* ps);
	virtual void sort_particle_system(particle_system* ps);
	virtual void draw_particle_system(particle_system* ps, const rtt::fbo* frame_buffer);
	
protected:	
	virtual void reset_particle_count(particle_system* ps);
	virtual void compute_particle_count(particle_system* ps);
	
	cl::NDRange init_range_local;
	cl::NDRange respawn_range_local;
	cl::NDRange compute_range_local;
	cl::NDRange sort_range_local;
	cl::NDRange compute_distances_local;
	unsigned long long int local_lcm;
	unsigned int kernel_seed;
	
};

#endif
