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

#ifndef __A2E_LIGHT_HPP__
#define __A2E_LIGHT_HPP__

#include "global.hpp"
#include "core/core.hpp"

//! lighting routines
class light {
public:
	light(const float& x, const float& y, const float& z);
	light(const float3& pos) : light(pos.x, pos.y, pos.z) {}
	~light();
	
	enum class LIGHT_TYPE : unsigned int {
		//! point light with specific radius (diff+spec only, no ambient color)
		POINT,
		//! TODO: !
		SPOT,
		//! global directional light (no attenuation, can use ambient color)
		DIRECTIONAL
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
	LIGHT_TYPE type = LIGHT_TYPE::POINT;
	float3 position;
	float3 color;
	float3 ambient;
	float3 spot_dir = float3(0.0f, 1.0f, 0.0f);
	float cutoff = 180.0f;
	float radius = 1.0f;
	float sqr_radius = 0.0f;
	float inv_sqr_radius = 0.0f;
	bool spot_light = false;
	bool enabled = true;

};

#endif
