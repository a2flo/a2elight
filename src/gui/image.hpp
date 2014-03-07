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

#ifndef __A2E_IMAGE_HPP__
#define __A2E_IMAGE_HPP__

#include "global.hpp"

#include "engine.hpp"

class A2E_API a2e_image {
public:
	constexpr a2e_image() = default;
	a2e_image(const string& filename, const TEXTURE_FILTERING filter = TEXTURE_FILTERING::POINT);
	a2e_image(a2e_image&& img) = default;
	~a2e_image() = default;
	
	a2e_image& operator=(a2e_image& img) = default;
	a2e_image& operator=(a2e_image&& img) = default;
	
	void open_image(const string& filename, const TEXTURE_FILTERING filter = TEXTURE_FILTERING::POINT);

	void draw();
	void draw(const pnt& scale_xy, const bool flip_y = false);

	void set_position(const unsigned int& x, const unsigned int& y);
	void set_position(const pnt& position);
	const pnt& get_position() const;

	const a2e_texture& get_texture() const;
	void set_texture(const a2e_texture& new_tex);

	void set_scaling(const bool& state);
	const bool& get_scaling() const;
	void set_gui_image(const bool& state);
	const bool& is_gui_image() const;

	unsigned int get_width() const;
	unsigned int get_height() const;

	void set_color(const float4& color);
	const float4& get_color() const;

protected:
	pnt position;
	float4 color { 1.0f };

	bool scale { true };
	bool gui_img { false };

	a2e_texture tex;

};

#endif
