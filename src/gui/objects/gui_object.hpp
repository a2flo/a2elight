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

#ifndef __A2E_GUI_OBJECT_HPP__
#define __A2E_GUI_OBJECT_HPP__

#include "global.hpp"
#include "math/vector_lib.hpp"
#include "gui/objects/gui_object_event.hpp"

// since all inheriting classes will need this, include it here:
#include "gui/style/gui_theme.hpp"
#include "gui/gui_event.hpp"

class gui;
class gui_window;
class a2e_image;

//! gui object element functions, base class for all gui objects
class gui_object : public gui_object_event {
public:
	gui_object() = delete;
	explicit gui_object(const float2& size, const float2& position) noexcept;
	virtual ~gui_object();
	
	virtual void draw() = 0;
	
	// state functions
	virtual void redraw();
	virtual bool needs_redraw() const;
	
	virtual void set_visible(const bool& visible);
	virtual void set_enabled(const bool& enabled);
	virtual void set_active(const bool& state);
	
	virtual bool is_visible() const;
	virtual bool is_enabled() const;
	virtual bool is_active() const;
	
	//
	virtual const float2& get_position() const;
	virtual const float2& get_position_abs() const;
	virtual const float2& get_size() const;
	virtual const float2& get_size_abs() const;
	virtual const rect& get_rectangle_abs() const;
	// use this for all abs values that need to be computed from normalized values
	virtual void compute_abs_values();
	
	virtual void set_position(const float2& position);
	virtual void set_size(const float2& size);
	
	//
	enum class FIXED_SCALE_DIRECTION {
		SCALE_WIDTH,
		SCALE_HEIGHT,
	};
	//! sets the width/height ratio at which the size of this object is computed (and scaled according to dir).
	//! NOTE: this must still be enabled via use_fixed_size_ratio(true)
	virtual void set_fixed_size_ratio(const float2& ratio, const FIXED_SCALE_DIRECTION& dir);
	virtual const float2& get_fixed_size_ratio() const;
	virtual const FIXED_SCALE_DIRECTION& get_fixed_size_ratio_scale_direction() const;
	virtual void use_fixed_size_ratio(const bool& state);
	virtual bool is_fixed_size_ratio() const;
	
	//
	enum class ATTACHMENT_SIDE {
		TOP,
		RIGHT,
		BOTTOM,
		LEFT,
	};
	//! attaches this gui object to the given obj, on the given side, placed at the given margin.
	//! to disable the attachment again, set obj to nullptr.
	//! NOTE: ring dependencies are not harmful right now, but results are probably not right
	virtual void set_attachment(gui_object* obj,
								const ATTACHMENT_SIDE side = ATTACHMENT_SIDE::RIGHT,
								const bool relative_margin = true,
								// if relative: in [0, 1]; if absolute, this is in px
								const float margin = 0.0f);
	virtual gui_object* get_attachment_object() const;
	virtual const ATTACHMENT_SIDE& get_attachment_side() const;
	virtual const float& get_attachment_margin() const;
	virtual const bool& is_attachment_relative_margin() const;
	virtual void compute_attachment_values();
	
	//
	virtual void set_parent(gui_object* parent);
	virtual gui_object* get_parent() const;
	virtual gui_window* get_parent_window() const;
	virtual void add_child(gui_object* child);
	virtual void remove_child(gui_object* child);
	virtual const vector<gui_object*>& get_children() const;
	virtual ipnt abs_to_rel_position(const ipnt& point) const;
	virtual ipnt rel_to_abs_position(const ipnt& point) const;
	
	//! sets the image associated with "identifier" for this object.
	//! NOTE: "#" is the default identifier that is used in most ui object layouts
	virtual void set_image(a2e_image* img, const string identifier = "#");
	virtual a2e_image* get_image(const string& identifier) const;
	
	// must return true if event was handled, false if not!
	virtual bool should_handle_mouse_event(const EVENT_TYPE& type, const ipnt& point) const;

protected:
	gui* ui;
	gui_theme* theme;
	font_manager* fm;
	a2e_font* fnt;
	
	// returns true if object should be drawn, false if it shouldn't; also resets the redraw flag
	virtual bool handle_draw();
	
	// gui object element variables
	struct {
		atomic<bool> visible { true };
		atomic<bool> enabled { true };
		atomic<bool> active { false };
		atomic<bool> redraw { true };
		atomic<bool> fixed_size_ratio { false };
	} state;
	
	float2 size; //!< normalized size (in [0, 1])
	float2 size_abs; //!< absolute screen space size
	float2 position; //!< normalized position (in [0, 1])
	float2 position_abs; //!< absolute screen coordinates
	float2 fixed_size_ratio { 1.0f, 1.0f }; //!< fixed size width/height ratio
	rect rectangle_abs; //!< computed absolute rectangle from position_abs and size_abs
	//! scale direction in which fixed_size_ratio is applied ("scale width or height according to the ratio")
	FIXED_SCALE_DIRECTION fixed_scale_direction { FIXED_SCALE_DIRECTION::SCALE_WIDTH };
	
	//
	gui_object* attached_object { nullptr };
	ATTACHMENT_SIDE attachment_side { ATTACHMENT_SIDE::RIGHT };
	bool attachment_relative_margin { true };
	float attachment_margin { 0.0f };
	
	//
	gui_object* parent { nullptr };
	vector<gui_object*> children;
	
	virtual bool is_window() const { return false; }
	
	// <identifier, image>
	unordered_map<string, a2e_image*> images;

};

#endif
