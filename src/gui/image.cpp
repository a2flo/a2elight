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

#include "image.hpp"
#include "rendering/gfx2d.hpp"

a2e_image::a2e_image(const string& filename, const TEXTURE_FILTERING filter) {
	open_image(filename, filter);
}

void a2e_image::draw(const uint2& scale_xy, const bool flip_y) {
	if(!gui_img) engine::start_2d_draw();
	
	if(tex->alpha) {
		glEnable(GL_BLEND);
	}
	
	float2 bottom_left, top_right;
	if(flip_y) {
		bottom_left.set(0.0f, 1.0f);
		top_right.set(1.0f, 0.0f);
	}
	else {
		bottom_left.set(0.0f, 0.0f);
		top_right.set(1.0f, 1.0f);
	}

	rect rectangle;
	if(scale) {
		rectangle.x1 = position.x;
		rectangle.y1 = position.y;
		rectangle.x2 = position.x + scale_xy.x;
		rectangle.y2 = position.y + scale_xy.y;
	}
	else {
		rectangle.x1 = position.x;
		rectangle.y1 = position.y;
		rectangle.x2 = position.x + (unsigned int)tex->width;
		rectangle.y2 = position.y + (unsigned int)tex->height;
	}
	gfx2d::draw_rectangle_texture(rectangle, tex->tex(), color, float4(0.0f), bottom_left, top_right);

	if(tex->alpha) { glDisable(GL_BLEND); }

	// if we want to draw 3d stuff later on, we have to clear
	// the depth buffer, otherwise nothing will be seen
	glClear(GL_DEPTH_BUFFER_BIT);

	if(!gui_img) engine::stop_2d_draw();
}

void a2e_image::draw() {
	draw((unsigned int)tex->width, (unsigned int)tex->height);
}

/*! opens an image file
 *  @param filename the image files name
 */
void a2e_image::open_image(const string& filename, const TEXTURE_FILTERING filter) {
	tex = engine::get_texman()->add_texture(filename, filter);
}

/*! sets the position (2 * unsigned int) of the image
 *  @param x the (new) x position of the image
 *  @param y the (new) y position of the image
 */
void a2e_image::set_position(const unsigned int& x, const unsigned int& y) {
	position.x = x;
	position.y = y;
}

/*! sets the position (pnt) of the image
 *  @param position the (new) position of the image
 */
void a2e_image::set_position(const uint2& position_) {
	set_position(position_.x, position_.y);
}

/*! returns the position (pnt) of the image
 */
const uint2& a2e_image::get_position() const {
	return position;
}

/*! sets the images texture
 *  @param tex the texture we want to set
 */
void a2e_image::set_texture(const a2e_texture& new_tex) {
	tex = new_tex;
}

//! returns the images texture
const a2e_texture& a2e_image::get_texture() const {
	return tex;
}

/*! sets image scaling to state
 *  @param state the scaling state
 */
void a2e_image::set_scaling(const bool& state) {
	a2e_image::scale = state;
}

//! returns the image scale flag
const bool& a2e_image::get_scaling() const {
	return scale;
}

void a2e_image::set_gui_image(const bool& state) {
	gui_img = state;
}

const bool& a2e_image::is_gui_image() const {
	return scale;
}

unsigned int a2e_image::get_width() const {
	return (unsigned int)tex->width;
}

unsigned int a2e_image::get_height() const {
	return (unsigned int)tex->height;
}

void a2e_image::set_color(const float4& color_) {
	color = color_;
}

const float4& a2e_image::get_color() const {
	return color;
}
