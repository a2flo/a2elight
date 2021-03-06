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

#ifndef __A2E_PARTICLE_BASE_HPP__
#define __A2E_PARTICLE_BASE_HPP__

#include "global.hpp"

#include <floor/core/core.hpp>
#include "engine.hpp"
#include "rendering/rtt.hpp"
#include "rendering/shader.hpp"
#include "rendering/texman.hpp"
#include "particle/particle_system.hpp"

class opencl;

//! a2e particle manager base class
class particle_manager_base {
public:
	particle_manager_base();
	virtual ~particle_manager_base();
	
	virtual void draw(const rtt::fbo* frame_buffer);
	virtual void run();
	
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
												 void* aux_data) = 0;
	virtual void delete_particle_system(particle_system* ps);
	virtual void reset_particle_system(particle_system* ps) = 0;
	virtual void run_particle_system(particle_system* ps) = 0;
	virtual void draw_particle_system(particle_system* ps, const rtt::fbo* frame_buffer) = 0;
	
protected:
	shader* s;
	rtt* r;
	ext* exts;
	texman* t;
	
	virtual particle_system* init(const particle_system::EMITTER_TYPE type,
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
	virtual void reset_particle_count(particle_system* ps) = 0;
	virtual void compute_particle_count(particle_system* ps);
	
	unsigned long long int max_particle_count;
	
	set<particle_system*> particle_systems;
	
};

#endif
