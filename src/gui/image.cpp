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

image::image() : t(engine::get_texman()), position() {
	scale = true;
	gui_img = false;
	color = 0xFFFFFF;
}

image::~image() {
}

/*! draws the image
 */
void image::draw(unsigned int scale_x, unsigned int scale_y, bool flip_y) {
	if(!gui_img) engine::start_2d_draw();
	
	const float4 fcolor(float((color>>16) & 0xFF) / 255.0f,
						float((color>>8) & 0xFF) / 255.0f,
						float(color & 0xFF) / 255.0f,
						1.0f);
	
	if(tex->alpha) {
		glEnable(GL_BLEND);
	}
	
	coord bottom_left, top_right;
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
		rectangle.x2 = position.x + scale_x;
		rectangle.y2 = position.y + scale_y;
	}
	else {
		rectangle.x1 = position.x;
		rectangle.y1 = position.y;
		rectangle.x2 = position.x + tex->width;
		rectangle.y2 = position.y + tex->height;
	}
	gfx2d::draw_rectangle_texture(rectangle, tex->tex(), fcolor, float4(0.0f), bottom_left, top_right);

	if(tex->alpha) { glDisable(GL_BLEND); }

	// if we want to draw 3d stuff later on, we have to clear
	// the depth buffer, otherwise nothing will be seen
	glClear(GL_DEPTH_BUFFER_BIT);

	if(!gui_img) engine::stop_2d_draw();
}

/*! draws the image
 */
void image::draw() {
	draw(tex->width, tex->height);
}

/*! opens an image file
 *  @param filename the image files name
 */
void image::open_image(const char* filename) {
	tex = t->add_texture(filename, TEXTURE_FILTERING::POINT);
}

/*! sets the position (2 * unsigned int) of the image
 *  @param x the (new) x position of the image
 *  @param y the (new) y position of the image
 */
void image::set_position(unsigned int x, unsigned int y) {
	position.x = x;
	position.y = y;
}

/*! sets the position (pnt) of the image
 *  @param position the (new) position of the image
 */
void image::set_position(pnt* position_) {
	set_position(position_->x, position_->y);
}

/*! returns the position (pnt) of the image
 */
pnt& image::get_position() {
	return position;
}

/*! sets the images texture
 *  @param tex the texture we want to set
 */
void image::set_texture(const a2e_texture& new_tex) {
	tex = new_tex;
}

//! returns the images texture
const a2e_texture& image::get_texture() const {
	return tex;
}

/*! sets image scaling to state
 *  @param state the scaling state
 */
void image::set_scaling(bool state) {
	image::scale = state;
}

//! returns the image scale flag
bool image::get_scaling() {
	return scale;
}

unsigned int image::get_width() {
	return tex->width;
}

unsigned int image::get_height() {
	return tex->height;
}

void image::set_color(unsigned int color_) {
	image::color = color_;
}

void image::set_gui_img(bool state) {
	gui_img = state;
}
