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

#include "gui/compound/gui_file_open_dialog.hpp"
#include "gui/objects/gui_button.hpp"
#include "gui/objects/gui_input_box.hpp"

gui_file_open_dialog::gui_file_open_dialog(const string& directory,
										   const string file_extension_,
										   const string file_description_,
										   function<bool(const string&)> file_filter_)
: gui_file_dialog(directory, file_extension_, file_description_, file_filter_) {
	open(directory);
}

gui_file_open_dialog::gui_file_open_dialog(const string& directory,
										   const file_io::FILE_TYPE& file_type_,
										   function<bool(const string&)> file_filter_)
: gui_file_dialog(directory, file_type_, file_filter_) {
	open(directory);
}

gui_file_open_dialog::~gui_file_open_dialog() {
}

void gui_file_open_dialog::open(const string directory, const bool write_history) {
	gui_file_dialog::open(directory, write_history);
	action_button->set_label("Open");
	
	if(!action_button->has_handler(GUI_EVENT::BUTTON_PRESS)) {
		action_button->add_handler([this](GUI_EVENT, gui_object&) {
			set_file_name();
			if(file_name.empty()) return;
			handle(GUI_EVENT::FILE_OPEN);
			
			floor::acquire_context();
			close();
			floor::release_context();
		}, GUI_EVENT::BUTTON_PRESS);
	}
}
