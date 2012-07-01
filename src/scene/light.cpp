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

#include "light.h"
#include "engine.h"

/*! creates a new light object
 *  @param e pointer to the engine class
 *  @param x the lights x position
 *  @param y the lights y position
 *  @param z the lights z position
 */
light::light(const float& x, const float& y, const float& z) 
: type(LT_POINT)
{
	position.set(x, y, z);
	color.set(0.0f, 0.0f, 0.0f);
	ambient.set(0.0f, 0.0f, 0.0f);

	spot_dir.set(0.0f, 1.0f, 0.0f);
	cutoff = 180.0f;
	
	enabled = true;
	spot_light = false;
	
	set_radius(1.0f);
}

/*! destroys the light object
 */
light::~light() {
}

void light::set_type(const LIGHT_TYPE& type_) {
	type = type_;
}

/*! sets the lights position
 *  @param x the lights x position
 *  @param y the lights y position
 *  @param z the lights z position
 */
void light::set_position(const float& x, const float& y, const float& z) {
	position.set(x, y, z);
}

/*! sets the lights position
 *  @param pos the lights position
 */
void light::set_position(const float3& pos) {
	set_position(pos.x, pos.y, pos.z);
}

/*! sets the lights color
 */
void light::set_color(const float& r, const float& g, const float& b) {
	color.set(r, g, b);
}

/*! sets the lights color
 *  @param amb the lights color
 */
void light::set_color(const float3& col) {
	set_color(col.x, col.y, col.z);
}

/*! sets the ambient lights color
 */
void light::set_ambient(const float& r, const float& g, const float& b) {
	ambient.set(r, g, b);
}

/*! sets the lights ambient color
 *  @param amb the lights ambient color
 */
void light::set_ambient(const float3& col) {
	set_ambient(col.x, col.y, col.z);
}

/*! sets the lights spot light direction
 *  @param direction the spots direction (float[3])
 */
void light::set_spot_direction(const float& dx, const float& dy, const float& dz) {
	spot_dir.set(dx, dy, dz);
}

/*! sets the lights spot light direction
 *  @param sdir the spots direction
 */
void light::set_spot_direction(const float3& sdir) {
	set_spot_direction(sdir.x, sdir.y, sdir.z);
}

/*! sets the lights spot light cutoff angle
 *  @param angle the spots cutoff angle (float)
 */
void light::set_spot_cutoff(const float& angle) {
	cutoff = angle;
}

/*! sets the lights spot light flag
 *  @param state the flag if the light is a spot light or not (bool)
 */
void light::set_spot_light(const bool& state) {
	spot_light = state;
}

/*! sets the lights enabled flag
 *  @param state the flag if the light is enabled or not (bool)
 */
void light::set_enabled(const bool& state) {
	enabled = state;
}

/*! sets the lights radius
 */
void light::set_radius(const float& radius_) {
	radius = radius_;
	sqr_radius = radius * radius;
	inv_sqr_radius = 1.0f / sqr_radius;
}

const light::LIGHT_TYPE& light::get_type() const {
	return type;
}

/*! returns the lights position
 */
const float3& light::get_position() const {
	return position;
}

/*! returns the lights color
 */
const float3& light::get_color() const {
	return color;
}

/*! returns the lights ambient color
 */
const float3& light::get_ambient() const {
	return ambient;
}

/*! returns the lights spot direction
 */
const float3& light::get_spot_direction() const {
	return spot_dir;
}

/*! returns the spot cutoff
 */
const float& light::get_spot_cutoff() const {
	return cutoff;
}

/*! returns the radius
 */
const float& light::get_radius() const {
	return radius;
}

const float& light::get_sqr_radius() const {
	return sqr_radius;
}

const float& light::get_inv_sqr_radius() const {
	return inv_sqr_radius;
}

/*! returns true if the light is a spot light
 */
bool light::is_spot_light() const {
	return spot_light;
}

/*! returns true if the light is enabled
 */
bool light::is_enabled() const {
	return enabled;
}
