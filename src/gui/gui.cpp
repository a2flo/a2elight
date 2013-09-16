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
main_fbo(1),
key_handler_fnctr(this, &gui::key_handler),
mouse_handler_fnctr(this, &gui::mouse_handler),
shader_reload_fnctr(this, &gui::shader_reload_handler),
window_handler_fnctr(this, &gui::window_handler) {
	// init clipboard (before adding event handlers)
	if(SDL_HasClipboardText()) clipboard_text = SDL_GetClipboardText();
	
	// create keyboard/mouse event handlers
	// note: the events will be deferred from the handlers
	// -> make the handlers internal, so events don't get deferred twice
	evt->add_internal_event_handler(key_handler_fnctr,
									EVENT_TYPE::KEY_DOWN,
									EVENT_TYPE::KEY_UP,
									EVENT_TYPE::UNICODE_INPUT,
									EVENT_TYPE::CLIPBOARD_UPDATE);
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
	log_debug("deleting gui object");
	
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

	log_debug("gui object deleted");
}

void gui::reload_shaders() {
	blend_shd = s->get_gl3shader("BLEND");
	texture_shd = s->get_gl3shader("GFX2D_TEXTURE");
}

void gui::draw() {
	gl_timer::mark("GUI_START");
	
	//
	glEnable(GL_BLEND);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, aa_fbo->draw_width, aa_fbo->draw_height);
	glDepthFunc(GL_LEQUAL);
	gfx2d::set_blend_mode(gfx2d::BLEND_MODE::PRE_MUL);
	
	//////////////////////////////////////////////////////////////////
	// draw individual surfaces
	
	// draw surfaces that need a redraw
	for(const auto& cb_surface : cb_surfaces) {
		if(cb_surface.second->needs_redraw()) cb_surface.second->draw();
	}
	
	// draw windows
	// try_lock should prevent any dead-locking due to other threads having the gui lock
	// and also trying to get the engine lock; in addition, this should also lead to
	// smoother gui drawing (-> no halts due to event handling)
	if(try_lock()) {
		for(const auto& wnd : windows) {
			wnd->draw();
		}
		unlock();
	}
	
	//////////////////////////////////////////////////////////////////
	// blit all surfaces onto gui buffer
	r->start_draw(&main_fbo);
	r->clear();
	r->start_2d_draw();
	
	gfx2d::set_blend_mode(gfx2d::BLEND_MODE::PRE_MUL);
	texture_shd->use();
	texture_shd->uniform("mvpm", *e->get_mvp_matrix());
	texture_shd->uniform("orientation", float4(0.0f, 0.0f, 1.0f, 1.0f));
	
	// pre ui callbacks
	for(auto& cb : draw_callbacks[0]) {
		cb_surfaces[cb]->blit(texture_shd);
	}
	
	// blit windows (in reverse order)
	// TODO: lock?
	for(auto riter = windows.crbegin(); riter != windows.crend(); riter++) {
		if((*riter)->is_visible()) (*riter)->blit(texture_shd);
	}
	
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
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
	
	//////////////////////////////////////////////////////////////////
	// blend with scene buffer and draw result to the main framebuffer
	e->start_2d_draw();
	
	blend_shd->use();
	blend_shd->texture("src_buffer", main_fbo.tex[0]);
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
			bool handled = false;
			for(const auto& wnd : windows) {
				if(wnd->handle_mouse_event(gevt.first, gevt.second, check_point)) {
					handled = true;
					break;
				}
			}
			
			// if no gui object handled the event, do some additional handling:
			if(!handled) {
				//
				switch(gevt.first) {
					case EVENT_TYPE::MOUSE_LEFT_DOWN:
						// if no object handled the mouse down/click event,
						// no object can be active -> deactivate active object
						set_active_object(nullptr);
						break;
					default: break;
				}
			}
		}
		// key events:
		else if((gevt.first & EVENT_TYPE::__KEY_EVENT) == EVENT_TYPE::__KEY_EVENT) {
			// TODO: key events should only be sent to the active window
			/*if(!windows.empty()) {
				windows[0]->handle_key_event(gevt.first, gevt.second);
			}*/
			// for now: send to all windows
			for(const auto& wnd : windows) {
				if(wnd->handle_key_event(gevt.first, gevt.second)) {
					break;
				}
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
	if(type == EVENT_TYPE::CLIPBOARD_UPDATE) {
		clipboard_text = ((const clipboard_update_event&)*obj).text;
		return true;
	}
	
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

bool gui::shader_reload_handler(EVENT_TYPE type, shared_ptr<event_object> obj floor_unused) {
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
		log_error("this ui draw callback already exists!");
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
		log_error("no such ui draw callback does exist!");
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
	
	// create fullscreen aa and main gui buffer
	aa_fbo = r->add_buffer((unsigned int)size.x, (unsigned int)size.y, GL_TEXTURE_2D, TEXTURE_FILTERING::POINT, e->get_ui_anti_aliasing(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 1, rtt::DEPTH_TYPE::RENDERBUFFER);
	
	glGenFramebuffers(1, &main_fbo.fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, main_fbo.fbo_id);
	
	main_fbo.width = aa_fbo->width;
	main_fbo.height = aa_fbo->height;
	main_fbo.draw_width = aa_fbo->draw_width;
	main_fbo.draw_height = aa_fbo->draw_height;
	main_fbo.tex[0] = aa_fbo->tex[0];
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, main_fbo.tex[0], 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, A2E_DEFAULT_FRAMEBUFFER);
	
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
	// note: main_fbo and aa_fbo share buffer data, so only delete the fbo of main_fbo
	if(main_fbo.fbo_id != 0) {
		main_fbo.tex[0] = 0;
		r->delete_buffer(&main_fbo);
	}
	if(aa_fbo != nullptr) {
		r->delete_buffer(aa_fbo);
		aa_fbo = nullptr;
	}
}

font_manager* gui::get_font_manager() const {
	return fm;
}

gui_theme* gui::get_theme() const {
	return theme;
}

void gui::add_window(gui_window* wnd) {
	windows.emplace_front(wnd);
}

void gui::remove_window(gui_window* wnd) {
	const auto iter = find(begin(windows), end(windows), wnd);
	if(iter != end(windows)) {
		windows.erase(iter);
	}
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

const rtt::fbo* gui::get_fullscreen_fbo() const {
	return aa_fbo;
}

bool gui::set_clipboard_text(const string& text) {
	if(SDL_SetClipboardText(text.c_str()) != 0) {
		log_error("couldn't set clipboard text: %s!", SDL_GetError());
		return false;
	}
	clipboard_text = text;
	return true;
}

const string& gui::get_clipboard_text() const {
	return clipboard_text;
}

void gui::set_active_object(gui_object* obj) {
	gui_object* old_active_object = active_object;
	active_object = obj;
	
	// deactivate old object
	if(old_active_object != nullptr) {
		old_active_object->set_active(false);
	}
	
	// activate new object
	if(active_object != nullptr) {
		active_object->set_active(true);
	}
}

gui_object* gui::get_active_object() const {
	return active_object;
}
