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
#include "gui/font_manager.h"
#include "gui/style/gui_theme.h"
#include "gui/objects/gui_window.h"

gui::gui(engine* e_, const string& theme_name) :
thread_base("gui"),
e(e_), evt(e_->get_event()), r(e_->get_rtt()), s(e_->get_shader()), sce(e_->get_scene()),
fm(new font_manager(e)),
theme(new gui_theme(e, fm)),
key_handler_fnctr(this, &gui::key_handler),
mouse_handler_fnctr(this, &gui::mouse_handler),
shader_reload_fnctr(this, &gui::shader_reload_handler),
window_handler_fnctr(this, &gui::window_handler) {
	// create keyboard/mouse event handlers
	// note: the events will be deferred from the handlers
	// -> make the handlers internal, so events don't get deferred twice
	evt->add_internal_event_handler(key_handler_fnctr,
									EVENT_TYPE::KEY_DOWN,
									EVENT_TYPE::KEY_UP);
	evt->add_internal_event_handler(mouse_handler_fnctr,
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
	
	// load theme
	theme->load("gui/"+theme_name+"/"+theme_name+".a2etheme");
	
	//
	evt->add_internal_event_handler(window_handler_fnctr, EVENT_TYPE::WINDOW_RESIZE);
	evt->add_internal_event_handler(shader_reload_fnctr, EVENT_TYPE::SHADER_RELOAD);
	reload_shaders();
	
	// start actual gui thread after everything has been initialized
	this->set_thread_delay(0);
	this->start();
}

gui::~gui() {
	a2e_debug("deleting gui object");
	
	set_thread_should_finish();
	
	evt->remove_event_handler(key_handler_fnctr);
	evt->remove_event_handler(mouse_handler_fnctr);
	evt->remove_event_handler(shader_reload_fnctr);
	evt->remove_event_handler(window_handler_fnctr);
	
	delete_buffers();
	
	finish();
	
	for(const auto& surface : cb_surfaces) {
		delete surface.second;
	}
	
	for(const auto& wnd : windows) {
		delete wnd;
	}
	
	delete theme;
	delete fm;

	a2e_debug("gui object deleted");
}

void gui::reload_shaders() {
	blend_shd = s->get_gl3shader("BLEND");
	texture_shd = s->get_gl3shader("GFX2D_TEXTURE");
}

void gui::draw() {
	gl_timer::mark("GUI_START");
	
	//
	glEnable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	gfx2d::set_blend_mode(gfx2d::BLEND_MODE::PRE_MUL);
	
	//////////////////////////////////////////////////////////////////
	// draw individual surfaces
	
	// draw surfaces that need a redraw
	for(const auto& cb_surface : cb_surfaces) {
		if(cb_surface.second->needs_redraw()) cb_surface.second->draw();
	}
	
	// draw windows
	lock();
	for(const auto& wnd : windows) {
		wnd->draw();
	}
	unlock();
	
	//////////////////////////////////////////////////////////////////
	// blit all surfaces onto gui buffer
	r->start_draw(main_fbo);
	r->clear();
	r->start_2d_draw();
	
	gfx2d::set_blend_mode(gfx2d::BLEND_MODE::ADD);
	texture_shd->use("passthrough");
	texture_shd->uniform("mvpm", *e->get_mvp_matrix());
	texture_shd->uniform("orientation", float4(0.0f, 0.0f, 1.0f, 1.0f));
	
	// pre ui callbacks
	for(auto& cb : draw_callbacks[0]) {
		cb_surfaces[cb]->blit(texture_shd);
	}
	
	// blit windows
	lock();
	for(const auto& wnd : windows) {
		wnd->blit(texture_shd);
	}
	unlock();
	
	// post ui callbacks
	for(auto& cb : draw_callbacks[1]) {
		cb_surfaces[cb]->blit(texture_shd);
	}
	
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	texture_shd->disable();
	
	// stop
	r->stop_2d_draw();
	r->stop_draw();
	
	//
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
	
	//////////////////////////////////////////////////////////////////
	// blend with scene buffer and draw result to the main framebuffer
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
	if(!events_in_queue) return;
	
	// copy/swap events from the event queue to the processing queue
	event_lock.lock();
	event_processing_queue.swap(event_queue);
	events_in_queue = false;
	event_lock.unlock();
	
	// lock gui
	lock();
	
	//
	if(windows.empty()) {
		unlock();
		event_processing_queue.clear();
		return;
	}
	
	// lock all windows
	for(const auto& wnd : windows) {
		wnd->lock();
	}
	
	// handle events
	for(const auto& gevt : event_processing_queue) {
		// mouse events:
		if((gevt.first & EVENT_TYPE::__MOUSE_EVENT) == EVENT_TYPE::__MOUSE_EVENT) {
			// note: mouse down/up events only affect the draw state, mouse clicks do the actual
			// logic checking (e.g. a button was pressed), since they contain both positions
			ipnt check_point(-1, -1);
			switch(gevt.first) {
				case EVENT_TYPE::MOUSE_LEFT_DOWN:
				case EVENT_TYPE::MOUSE_LEFT_UP:
				case EVENT_TYPE::MOUSE_LEFT_HOLD:
				case EVENT_TYPE::MOUSE_RIGHT_DOWN:
				case EVENT_TYPE::MOUSE_RIGHT_UP:
				case EVENT_TYPE::MOUSE_RIGHT_HOLD:
				case EVENT_TYPE::MOUSE_MIDDLE_DOWN:
				case EVENT_TYPE::MOUSE_MIDDLE_UP:
				case EVENT_TYPE::MOUSE_MIDDLE_HOLD:
				case EVENT_TYPE::MOUSE_MOVE:
					check_point = ((const mouse_event_base<EVENT_TYPE::__MOUSE_EVENT>&)*gevt.second).position;
					break;
					
				case EVENT_TYPE::MOUSE_LEFT_CLICK:
				case EVENT_TYPE::MOUSE_LEFT_DOUBLE_CLICK:
				case EVENT_TYPE::MOUSE_RIGHT_CLICK:
				case EVENT_TYPE::MOUSE_RIGHT_DOUBLE_CLICK:
				case EVENT_TYPE::MOUSE_MIDDLE_CLICK:
				case EVENT_TYPE::MOUSE_MIDDLE_DOUBLE_CLICK:
					// check down, because up and down must both match (be within) in the target object
					// and the object will have already received a mouse down event
					// note: memory layout is the same for all click events (-> mouse_left_click_event)
					check_point = ((const mouse_left_click_event&)*gevt.second).down->position;
					break;
					
				case EVENT_TYPE::MOUSE_WHEEL_UP:
				case EVENT_TYPE::MOUSE_WHEEL_DOWN:
					// mouse wheel events actually also have a position and should only be sent to the
					// windows underneath that position (-> scrolling in inactive windows)
					check_point = ((const mouse_wheel_event_base<EVENT_TYPE::__MOUSE_EVENT>&)*gevt.second).position;
					break;
				default: break;
			}
			
			// check all windows
			for(const auto& wnd : windows) {
				if(gfx2d::is_pnt_in_rectangle(wnd->get_rectangle_abs(), check_point) &&
				   wnd->handle_mouse_event(gevt.first, gevt.second, check_point)) {
					break;
				}
			}
		}
		// key events:
		else if((gevt.first & EVENT_TYPE::__KEY_EVENT) == EVENT_TYPE::__KEY_EVENT) {
			// key events should only be sent to the active window
			if(!windows.empty()) {
				windows[0]->handle_key_event(gevt.first, gevt.second);
			}
		}
	}
	
	// unlock again
	for(const auto& wnd : windows) {
		wnd->unlock();
	}
	unlock();
	
	event_processing_queue.clear(); // *done*
}

bool gui::key_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(!keyboard_input) return false;
	event_lock.lock();
	event_queue.emplace_back(type, obj);
	events_in_queue = true;
	event_lock.unlock();
	return true;
}

bool gui::mouse_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(!mouse_input) return false;
	event_lock.lock();
	event_queue.emplace_back(type, obj);
	events_in_queue = true;
	event_lock.unlock();
	return true;
}

bool gui::shader_reload_handler(EVENT_TYPE type, shared_ptr<event_object> obj a2e_unused) {
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
	keyboard_input = state;
}

void gui::set_mouse_input(const bool& state) {
	mouse_input = state;
}

bool gui::get_keyboard_input() const {
	return keyboard_input;
}

bool gui::get_mouse_input() const {
	return mouse_input;
}

gui_simple_callback* gui::add_draw_callback(const DRAW_MODE_UI& mode, ui_draw_callback& cb,
											const float2& size, const float2& offset,
											const gui_surface::SURFACE_FLAGS flags) {
	auto& callbacks = draw_callbacks[mode == DRAW_MODE_UI::PRE_UI ? 0 : 1];
	const auto iter = find(begin(callbacks), end(callbacks), &cb);
	if(iter != end(callbacks)) {
		a2e_error("this ui draw callback already exists!");
		return nullptr;
	}
	callbacks.emplace_back(&cb);
	
	const auto surface_iter = cb_surfaces.find(&cb);
	if(surface_iter != cb_surfaces.end()) return surface_iter->second;
	
	gui_simple_callback* surface = new gui_simple_callback(cb, mode, e, size, offset, flags);
	cb_surfaces.insert(make_pair(&cb, surface));
	return surface;
}

void gui::delete_draw_callback(ui_draw_callback& cb) {
	const auto iter_0 = find(begin(draw_callbacks[0]), end(draw_callbacks[0]), &cb);
	const auto iter_1 = find(begin(draw_callbacks[1]), end(draw_callbacks[1]), &cb);
	
	if(iter_0 == end(draw_callbacks[0]) && iter_1 == end(draw_callbacks[1])) {
		a2e_error("no such ui draw callback does exist!");
		return;
	}
	
	if(iter_0 != end(draw_callbacks[0])) draw_callbacks[0].erase(iter_0);
	if(iter_1 != end(draw_callbacks[1])) draw_callbacks[1].erase(iter_1);
	
	const auto surface_iter = cb_surfaces.find(&cb);
	if(surface_iter != cb_surfaces.end()) {
		delete surface_iter->second;
		cb_surfaces.erase(surface_iter);
	}
}

void gui::recreate_buffers(const size2 size) {
	delete_buffers();
	
	// create main gui buffer
	main_fbo = r->add_buffer((unsigned int)size.x, (unsigned int)size.y, GL_TEXTURE_2D, TEXTURE_FILTERING::POINT, rtt::TEXTURE_ANTI_ALIASING::NONE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 1, rtt::DEPTH_TYPE::NONE);
	
	// resize/recreate surfaces
	for(const auto& surface : cb_surfaces) {
		surface.second->resize(surface.second->get_buffer_size());
	}
	
	// resize/recreate window surfaces
	for(const auto& wnd : windows) {
		wnd->resize(wnd->get_buffer_size());
	}
}

void gui::delete_buffers() {
	if(r != nullptr) {
		if(main_fbo != nullptr) {
			r->delete_buffer(main_fbo);
			main_fbo = nullptr;
		}
	}
}

font_manager* gui::get_font_manager() const {
	return fm;
}

gui_theme* gui::get_theme() const {
	return theme;
}

void gui::add_window(gui_window* wnd) {
	windows.emplace_back(wnd);
}

void gui::lock() {
	object_lock.lock();
}

bool gui::try_lock() {
	return object_lock.try_lock();
}

void gui::unlock() {
	object_lock.unlock();
}
