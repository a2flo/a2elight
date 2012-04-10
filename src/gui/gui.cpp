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

#include "gui.h"
#include "engine.h"
#include "rendering/shader.h"
#include "rendering/gfx2d.h"
#include "scene/scene.h"

gui::gui(engine* e_) :
thread_base("gui"),
e(e_), evt(e_->get_event()), r(e_->get_rtt()), s(e_->get_shader()), sce(e_->get_scene()),
main_fbo(nullptr),
key_handler_fnctr(this, &gui::key_handler),
mouse_handler_fnctr(this, &gui::mouse_handler),
shader_reload_fnctr(this, &gui::shader_reload_handler){
	AtomicSet(&keyboard_input, 1);
	AtomicSet(&mouse_input, 1);

	// create keyboard/mouse event handlers
	evt->add_event_handler(key_handler_fnctr,
						   EVENT_TYPE::KEY_DOWN,
						   EVENT_TYPE::KEY_UP,
						   EVENT_TYPE::KEY_PRESSED);
	evt->add_event_handler(mouse_handler_fnctr,
						   EVENT_TYPE::MOUSE_LEFT_DOWN,
						   EVENT_TYPE::MOUSE_LEFT_UP,
						   EVENT_TYPE::MOUSE_LEFT_CLICK,
						   EVENT_TYPE::MOUSE_LEFT_DOUBLE_CLICK,
						   EVENT_TYPE::MOUSE_LEFT_HOLD,
						   
						   EVENT_TYPE::MOUSE_RIGHT_DOWN,
						   EVENT_TYPE::MOUSE_RIGHT_UP,
						   EVENT_TYPE::MOUSE_RIGHT_CLICK,
						   EVENT_TYPE::MOUSE_RIGHT_DOUBLE_CLICK,
						   EVENT_TYPE::MOUSE_RIGHT_HOLD,
						   
						   EVENT_TYPE::MOUSE_MIDDLE_DOWN,
						   EVENT_TYPE::MOUSE_MIDDLE_UP,
						   EVENT_TYPE::MOUSE_MIDDLE_CLICK,
						   EVENT_TYPE::MOUSE_MIDDLE_DOUBLE_CLICK,
						   EVENT_TYPE::MOUSE_MIDDLE_HOLD,
						   
						   EVENT_TYPE::MOUSE_MOVE,
						   
						   EVENT_TYPE::MOUSE_WHEEL_UP,
						   EVENT_TYPE::MOUSE_WHEEL_DOWN);
	
	// create main gui buffer
	const auto gui_aa = rtt::TAA_MSAA_8; // TODO: config option
	const auto gui_filter = texture_object::TF_LINEAR;
	main_fbo = r->add_buffer(e->get_width(), e->get_height(), GL_TEXTURE_2D, gui_filter, gui_aa, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 1, rtt::DT_RENDERBUFFER);
	
	//
	evt->add_internal_event_handler(shader_reload_fnctr, EVENT_TYPE::SHADER_RELOAD);
	reload_shaders();
	
	// start actual gui thread after everything has been initialized
	this->start();
}

gui::~gui() {
	a2e_debug("deleting gui object");
	
	if(r != nullptr && main_fbo != nullptr) {
		r->delete_buffer(main_fbo);
	}
	
	evt->remove_event_handler(key_handler_fnctr);
	evt->remove_event_handler(mouse_handler_fnctr);
	evt->remove_event_handler(shader_reload_fnctr);

	a2e_debug("gui object deleted");
}

void gui::reload_shaders() {
	blend_shd = s->get_gl3shader("BLEND");
}

void gui::draw() {
	// draw to gui buffer
	r->start_draw(main_fbo);
	r->clear();
	r->start_2d_draw();
	glEnable(GL_BLEND);
	gfx2d::set_blend_mode(gfx2d::BLEND_MODE::PRE_MUL);
	
	// draw everything else (pre ui)
	for(auto& cb : draw_callbacks) {
		(*cb)(DRAW_MODE_UI::PRE_UI);
	}
	
	// TODO: draw windows
	
	// draw everything else (post ui)
	for(auto& cb : draw_callbacks) {
		(*cb)(DRAW_MODE_UI::POST_UI);
	}
	
	// stop
	glDisable(GL_BLEND);
	r->stop_2d_draw();
	r->stop_draw();
	
	// "blit" to main framebuffer
	e->start_2d_draw();
	
	blend_shd->use();
	blend_shd->texture("src_buffer", main_fbo->tex_id[0]);
	blend_shd->texture("dst_buffer", sce->_get_scene_buffer()->tex_id[0]);
	
	glFrontFace(GL_CW);
	gfx2d::draw_fullscreen_triangle();
	glFrontFace(GL_CCW);
	
	blend_shd->disable();
	
	e->stop_2d_draw();
}

void gui::run() {
	// TODO: event handling?
}

bool gui::key_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(!get_keyboard_input()) return false;
	return false;
}

bool gui::mouse_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(!get_mouse_input()) return false;
	//cout << "mouse event: " << (unsigned int)type << endl;
	return false;
}

bool gui::shader_reload_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(type == EVENT_TYPE::SHADER_RELOAD) {
		reload_shaders();
	}
	return true;
}

void gui::set_keyboard_input(const bool& state) {
	AtomicSet(&keyboard_input, state ? 1 : 0);
}

void gui::set_mouse_input(const bool& state) {
	AtomicSet(&mouse_input, state ? 1 : 0);
}

bool gui::get_keyboard_input() const {
	const int state = AtomicGet((atomic_t*)&keyboard_input);
	return (state == 0 ? false : true);
}

bool gui::get_mouse_input() const {
	const int state = AtomicGet((atomic_t*)&mouse_input);
	return (state == 0 ? false : true);
}

void gui::add_draw_callback(draw_callback& cb) {
	for(const auto& elem : draw_callbacks) {
		if(elem == &cb) {
			a2e_error("this ui draw callback already exists!");
			return;
		}
	}
	draw_callbacks.emplace_back(&cb);
}

void gui::delete_draw_callback(draw_callback& cb) {
	for(auto iter = draw_callbacks.cbegin(); iter != draw_callbacks.cend(); iter++) {
		if(*iter == &cb) {
			draw_callbacks.erase(iter);
			return;
		}
	}
	a2e_error("no such ui draw callback does exist!");
}
