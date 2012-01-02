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

#ifndef __RAY_H__
#define __RAY_H__

#include "core/cpp_headers.h"
#include "core/vector3.h"

class A2E_API ray {
public:
	float3 origin;
	float3 direction;
	
	ray() : origin(), direction() {}
	ray(const ray& r) : origin(r.origin), direction(r.direction) {}
	ray(const float3& rorigin, const float3& rdirection) : origin(rorigin), direction(rdirection) {}
	
	float3 get_point(const float& distance) {
		return origin + distance * direction;
	}
	
};

#endif
