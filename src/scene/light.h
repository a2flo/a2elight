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

#ifndef __A2E_LIGHT_H__
#define __A2E_LIGHT_H__

#include "global.h"
#include "core/core.h"

/*! @class light
 *  @brief lighting routines
 */

class engine;
class A2E_API light {
public:
	light(engine* e, const float& x, const float& y, const float& z);
	light(engine* e_, const float3& pos) : light(e_, pos.x, pos.y, pos.z) {}
	~light();
	
	enum LIGHT_TYPE {
		//! point light with specific radius (diff+spec only, no ambient color)
		LT_POINT,
		//! TODO: !
		LT_SPOT,
		//! global directional light (no attenuation, can use ambient color)
		LT_DIRECTIONAL
	};

	void set_type(const LIGHT_TYPE& type);
	void set_position(const float& x, const float& y, const float& z);
	void set_position(const float3& pos);
	void set_color(const float& r, const float& g, const float& b);
	void set_color(const float3& col);
	void set_ambient(const float& r, const float& g, const float& b);
	void set_ambient(const float3& col);
	void set_spot_direction(const float& dx, const float& dy, const float& dz);
	void set_spot_direction(const float3& sdir);
	void set_spot_cutoff(const float& angle);
	void set_radius(const float& radius);
	
	void set_enabled(const bool& state);
	void set_spot_light(const bool& state);

	const LIGHT_TYPE& get_type() const;
	const float3& get_position() const;
	const float3& get_color() const;
	const float3& get_ambient() const;
	const float3& get_spot_direction() const;
	const float& get_spot_cutoff() const;
	const float& get_radius() const;
	const float& get_sqr_radius() const;
	const float& get_inv_sqr_radius() const;
	bool is_spot_light() const;
	bool is_enabled() const;

protected:
	engine* e;

	LIGHT_TYPE type;
	float3 position;
	float3 color;
	float3 ambient;
	float3 spot_dir;
	float cutoff;
	float radius;
	float sqr_radius;
	float inv_sqr_radius;

	bool spot_light;
	bool enabled;

};

#endif
