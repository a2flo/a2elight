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

#ifndef __A2E_CAMERA_H__
#define __A2E_CAMERA_H__

#include "global.hpp"

#include "core/core.hpp"
#include "core/event.hpp"

/*! @class camera
 *  @brief a2e camera functions
 */

class engine;

class A2E_API camera {
public:
	camera(engine* e);
	~camera();

	void run();

	void set_position(const float& x, const float& y, const float& z);
	void set_position(const float3& pos);
	float3& get_position();
	const float3& get_position() const;
	
	void set_rotation(const float& x, const float& y, const float& z);
	void set_rotation(const float3& rot);
	float3& get_rotation();
	const float3& get_rotation() const;

	void set_rotation_speed(const float& speed);
	float get_rotation_speed() const;
	void set_cam_speed(const float& speed);
	float get_cam_speed() const;

	void set_keyboard_input(const bool& state);
	bool get_keyboard_input() const;
	void set_mouse_input(const bool& state);
	bool get_mouse_input() const;
	void set_wasd_input(const bool& state);
	bool get_wasd_input() const;

	const float3& get_direction();

protected:
	engine* e;
	event* evt;

	float3 position;
	float3 rotation;
	float3 direction;
	float up_down = 0.0f;
	float rotation_speed = 100.0f;
	float cam_speed = 1.0f;

	bool keyboard_input = true;
	bool mouse_input = false;
	bool wasd_input = false;
	unsigned int ignore_next_rotation = 0;
	
	// [right, left, up, down]
	array<bool, 4> key_state = { { false, false, false, false } };
	
	event::handler keyboard_handler;
	bool key_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	
};

#endif
