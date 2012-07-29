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
#include "rendering/gl_timer.h"

gui::gui(engine* e_) :
thread_base("gui"),
e(e_), evt(e_->get_event()), r(e_->get_rtt()), s(e_->get_shader()), sce(e_->get_scene()),
main_fbo(nullptr),
key_handler_fnctr(this, &gui::key_handler),
mouse_handler_fnctr(this, &gui::mouse_handler),
shader_reload_fnctr(this, &gui::shader_reload_handler),
window_handler_fnctr(this, &gui::window_handler) {
	AtomicSet(&keyboard_input, 1);
	AtomicSet(&mouse_input, 1);

	// create keyboard/mouse event handlers
	evt->add_event_handler(key_handler_fnctr,
						   EVENT_TYPE::KEY_DOWN,
						   EVENT_TYPE::KEY_UP);
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
	
	recreate_buffers(size2(e->get_width(), e->get_height()));
	
	//
	evt->add_internal_event_handler(window_handler_fnctr, EVENT_TYPE::WINDOW_RESIZE);
	evt->add_internal_event_handler(shader_reload_fnctr, EVENT_TYPE::SHADER_RELOAD);
	reload_shaders();
	
	// start actual gui thread after everything has been initialized
	this->set_thread_delay(50);
	this->start();
}

gui::~gui() {
	a2e_debug("deleting gui object");
	
	evt->remove_event_handler(key_handler_fnctr);
	evt->remove_event_handler(mouse_handler_fnctr);
	evt->remove_event_handler(shader_reload_fnctr);
	evt->remove_event_handler(window_handler_fnctr);
	
	delete_buffers();

	a2e_debug("gui object deleted");
}

void gui::reload_shaders() {
	blend_shd = s->get_gl3shader("BLEND");
}

void gui::draw() {
	gl_timer::mark("GUI_START");
	// draw to gui buffer
	r->start_draw(main_fbo);
	r->clear();
	r->start_2d_draw();
	glEnable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
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
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
	r->stop_2d_draw();
	r->stop_draw();
	
	// "blit" to main framebuffer
	e->start_2d_draw();
	
	blend_shd->use();
	blend_shd->texture("src_buffer", main_fbo->tex[0]);
	blend_shd->texture("dst_buffer", sce->get_scene_buffer()->tex[0]);
	
	glFrontFace(GL_CW);
	gfx2d::draw_fullscreen_triangle();
	glFrontFace(GL_CCW);
	
	blend_shd->disable();
	
	e->stop_2d_draw();
	gl_timer::mark("GUI_END");
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

bool gui::window_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(type == EVENT_TYPE::WINDOW_RESIZE) {
		const window_resize_event& evtobj = (const window_resize_event&)*obj;
		recreate_buffers(evtobj.size);
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
	for(auto iter = draw_callbacks.begin(); iter != draw_callbacks.end(); iter++) {
		if(*iter == &cb) {
			draw_callbacks.erase(iter);
			return;
		}
	}
	a2e_error("no such ui draw callback does exist!");
}

void gui::recreate_buffers(const size2 size) {
	delete_buffers();
	
	// create main gui buffer
	rtt::TEXTURE_ANTI_ALIASING gui_aa = rtt::TAA_MSAA_8;
	switch(e->get_ui_anti_aliasing()) {
		case 0: gui_aa = rtt::TAA_NONE; break;
		case 1: gui_aa = rtt::TAA_MSAA_1; break;
		case 2: gui_aa = rtt::TAA_MSAA_2; break;
		case 4: gui_aa = rtt::TAA_MSAA_4; break;
		case 8: gui_aa = rtt::TAA_MSAA_8; break;
		case 16: gui_aa = rtt::TAA_MSAA_16; break;
		case 32: gui_aa = rtt::TAA_MSAA_32; break;
		case 64: gui_aa = rtt::TAA_MSAA_64; break;
		default: break;
	}
	const auto gui_filter = texture_object::TF_LINEAR;
	main_fbo = r->add_buffer((unsigned int)size.x, (unsigned int)size.y, GL_TEXTURE_2D, gui_filter, gui_aa, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 1, rtt::DT_RENDERBUFFER);
}

void gui::delete_buffers() {
	if(r != nullptr && main_fbo != nullptr) {
		r->delete_buffer(main_fbo);
		main_fbo = nullptr;
	}
}
