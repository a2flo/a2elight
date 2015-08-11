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

#include "gui_object.hpp"
#include "engine.hpp"
#include "gui/gui.hpp"
#include "font_manager.hpp"

gui_object::gui_object(const float2& size_, const float2& position_) noexcept :
gui_object_event(),
ui(engine::get_gui()), theme(ui->get_theme()), fm(ui->get_font_manager()),
fnt(fm->get_font("SYSTEM_SANS_SERIF")), size(size_), position(position_) {
	compute_abs_values();
}

gui_object::~gui_object() {
	// if this is the active gui object, set the active object to nullptr
	if(state.active || ui->get_active_object() == this) {
		ui->set_active_object(nullptr);
	}
	// remove from parent
	if(parent != nullptr) {
		parent->remove_child(this);
	}
}

bool gui_object::handle_draw() {
	if(!state.visible) return false;
	state.redraw = false;
	return true;
}

void gui_object::redraw() {
	state.redraw = true;
}

bool gui_object::needs_redraw() const {
	return state.redraw;
}

void gui_object::set_visible(const bool& visible_state) {
	state.visible = visible_state;
}

void gui_object::set_enabled(const bool& enabled_state) {
	state.enabled = enabled_state;
}

void gui_object::set_active(const bool& active_state) {
	state.active = active_state;
	
	// in case the active state is set externally (not by an event),
	// we have to make sure the gui knows about this
	if((state.active && ui->get_active_object() != this) ||
	   (!state.active && ui->get_active_object() == this)) {
		ui->set_active_object(state.active ? this : nullptr);
	}
	
	redraw();
}

bool gui_object::is_visible() const {
	return state.visible;
}

bool gui_object::is_enabled() const {
	return state.enabled;
}

bool gui_object::is_active() const {
	return state.active;
}

void gui_object::compute_abs_values() {
	const float2 parent_size(parent != nullptr ?
							 parent->get_size_abs() :
							 float2(floor::get_physical_width(), floor::get_physical_height()));
	position_abs = position * parent_size;
	size_abs = size * parent_size;
	
	if(state.fixed_size_ratio) {
		if(fixed_scale_direction == FIXED_SCALE_DIRECTION::SCALE_WIDTH) {
			const float cur_ratio = size_abs.y / size_abs.x; // x: 1, y: *cur_ratio
			const float target_ratio = fixed_size_ratio.y / fixed_size_ratio.x;
			size_abs.x *= cur_ratio / target_ratio;
			size.x = size_abs.x / parent_size.x;
		}
		else { // FIXED_SCALE_DIRECTION::SCALE_HEIGHT
			const float cur_ratio = size_abs.x / size_abs.y; // x: *cur_ratio, y: 1
			const float target_ratio = fixed_size_ratio.x / fixed_size_ratio.y;
			size_abs.y *= cur_ratio / target_ratio;
			size.y = size_abs.y / parent_size.y;
		}
	}
	
	rectangle_abs.set((unsigned int)position_abs.x,
					  (unsigned int)position_abs.y,
					  (unsigned int)(position_abs.x + size_abs.x),
					  (unsigned int)(position_abs.y + size_abs.y));
}

const float2& gui_object::get_position() const {
	return position;
}

const float2& gui_object::get_position_abs() const {
	return position_abs;
}

const float2& gui_object::get_size() const {
	return size;
}

const float2& gui_object::get_size_abs() const {
	return size_abs;
}

const rect& gui_object::get_rectangle_abs() const {
	return rectangle_abs;
}

void gui_object::set_position(const float2& position_) {
	position = position_;
	compute_abs_values();
	compute_attachment_values();
	redraw();
}

void gui_object::set_size(const float2& size_) {
	size = size_;
	compute_abs_values();
	compute_attachment_values();
	redraw();
}

void gui_object::set_fixed_size_ratio(const float2& ratio, const FIXED_SCALE_DIRECTION& dir) {
	fixed_size_ratio = ratio;
	fixed_scale_direction = dir;
	if(state.fixed_size_ratio) {
		compute_abs_values();
		compute_attachment_values();
		redraw();
	}
}

const float2& gui_object::get_fixed_size_ratio() const {
	return fixed_size_ratio;
}

const gui_object::FIXED_SCALE_DIRECTION& gui_object::get_fixed_size_ratio_scale_direction() const {
	return fixed_scale_direction;
}

void gui_object::use_fixed_size_ratio(const bool& state_) {
	if(state_ != state.fixed_size_ratio) {
		state.fixed_size_ratio = state_;
		compute_abs_values();
		compute_attachment_values();
		redraw();
	}
}

bool gui_object::is_fixed_size_ratio() const {
	return state.fixed_size_ratio;
}

void gui_object::set_attachment(gui_object* obj, const ATTACHMENT_SIDE side,
								const bool relative_margin, const float margin) {
	attached_object = obj;
	attachment_side = side;
	attachment_relative_margin = relative_margin;
	attachment_margin = margin;
}

gui_object* gui_object::get_attachment_object() const {
	return attached_object;
}

const gui_object::ATTACHMENT_SIDE& gui_object::get_attachment_side() const {
	return attachment_side;
}

const float& gui_object::get_attachment_margin() const {
	return attachment_margin;
}

const bool& gui_object::is_attachment_relative_margin() const {
	return attachment_relative_margin;
}

void gui_object::compute_attachment_values() {
	if(attached_object == nullptr) return;
	
	const auto& att_size = attached_object->get_size();
	const auto& att_pos = attached_object->get_position();
	const auto& att_rect = attached_object->get_rectangle_abs();
	
	const float2 parent_size(parent != nullptr ?
							 parent->get_size_abs() :
							 float2(floor::get_physical_width(), floor::get_physical_height()));
	
	if(attachment_relative_margin) {
		switch(attachment_side) {
			case ATTACHMENT_SIDE::TOP:
				position.y = att_pos.y - attachment_margin - size.y;
				break;
			case ATTACHMENT_SIDE::RIGHT:
				position.x = att_pos.x + att_size.x + attachment_margin;
				break;
			case ATTACHMENT_SIDE::BOTTOM:
				position.y = att_pos.y + att_size.y + attachment_margin;
				break;
			case ATTACHMENT_SIDE::LEFT:
				position.x = att_pos.x - attachment_margin - size.x;
				break;
		}
		
		if(attachment_side == ATTACHMENT_SIDE::TOP ||
		   attachment_side == ATTACHMENT_SIDE::BOTTOM) {
			position_abs.y = position.y * parent_size.y;
		}
		else position_abs.x = position.x * parent_size.x;
	}
	else {
		// absolute margin
		switch(attachment_side) {
			case ATTACHMENT_SIDE::TOP:
				position_abs.y = float(att_rect.y1) - attachment_margin - size_abs.y;
				break;
			case ATTACHMENT_SIDE::RIGHT:
				position_abs.x = float(att_rect.x2) + attachment_margin;
				break;
			case ATTACHMENT_SIDE::BOTTOM:
				position_abs.y = float(att_rect.y2) + attachment_margin;
				break;
			case ATTACHMENT_SIDE::LEFT:
				position_abs.x = float(att_rect.x1) - attachment_margin - size_abs.x;
				break;
		}
		
		if(attachment_side == ATTACHMENT_SIDE::TOP ||
		   attachment_side == ATTACHMENT_SIDE::BOTTOM) {
			position.y = position_abs.y / parent_size.y;
		}
		else position.x = position_abs.x / parent_size.x;
	}
	
	rectangle_abs.set((unsigned int)position_abs.x,
					  (unsigned int)position_abs.y,
					  (unsigned int)(position_abs.x + size_abs.x),
					  (unsigned int)(position_abs.y + size_abs.y));
}

void gui_object::set_parent(gui_object* parent_) {
	lock();
	if(parent != nullptr) {
		parent->remove_child(this);
	}
	parent = parent_;
	compute_abs_values();
	compute_attachment_values();
	unlock();
}

gui_object* gui_object::get_parent() const {
	return parent;
}

gui_window* gui_object::get_parent_window() const {
	for(auto parent_ptr = parent; parent_ptr != nullptr; parent_ptr = parent_ptr->get_parent()) {
		if(parent_ptr->is_window()) {
			return (gui_window*)parent_ptr;
		}
	}
	return ui->get_main_window();
}

void gui_object::add_child(gui_object* child) {
	lock();
	const auto child_iter = find(children.cbegin(), children.cend(), child);
	if(child_iter == children.cend()) {
		children.emplace_back(child);
		child->set_parent(this);
	}
	unlock();
}

void gui_object::remove_child(gui_object* child) {
	lock();
	const auto iter = find(children.cbegin(), children.cend(), child);
	if(iter != children.cend()) {
		children.erase(iter);
		child->set_parent(nullptr);
	}
	unlock();
}

const vector<gui_object*>& gui_object::get_children() const {
	return children;
}

int2 gui_object::abs_to_rel_position(const int2& point) const {
	// override this in objects that contain other objects (e.g. gui_window)
	if(parent != nullptr) return parent->abs_to_rel_position(point);
	return point;
}

int2 gui_object::rel_to_abs_position(const int2& point) const {
	// override this in objects that contain other objects (e.g. gui_window)
	if(parent != nullptr) return parent->rel_to_abs_position(point);
	return point;
}

bool gui_object::should_handle_mouse_event(const EVENT_TYPE& type floor_unused, const int2& point) const {
	return gfx2d::is_pnt_in_rectangle(get_rectangle_abs(), point);
}

void gui_object::set_image(a2e_image* img, const string identifier) {
	images[identifier] = img;
	redraw();
}

a2e_image* gui_object::get_image(const string& identifier) const {
	const auto iter = images.find(identifier);
	if(iter == images.end()) {
		return nullptr;
	}
	return iter->second;
}
