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

#ifndef __A2E_PARTICLE_H__
#define __A2E_PARTICLE_H__

#include "global.hpp"

#include "core/core.hpp"
#include "engine.hpp"
#include "rendering/rtt.hpp"
#include "rendering/shader.hpp"
#include "rendering/texman.hpp"
#include "particle/particle_system.hpp"

/*! @class particle_manager
 *  @brief a2e particle manager
 */

class opencl;
class particle_manager_base;
class A2E_API particle_manager {
public:
	particle_manager(engine* e);
	~particle_manager();

	void draw(const rtt::fbo* frame_buffer);
	void draw_particle_system(particle_system* ps, const rtt::fbo* frame_buffer);
	void run();
	
	particle_system* add_particle_system(const particle_system::EMITTER_TYPE type,
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
										 void* aux_data = nullptr);
	void delete_particle_system(particle_system* ps);
	particle_manager_base* get_manager();
	void set_manager(particle_manager_base* pm);

protected:
	engine* e;
	shader* s;
	opencl_base* cl;
	rtt* r;
	ext* exts;
	texman* t;
	
	particle_manager_base* pm;

};

#endif
