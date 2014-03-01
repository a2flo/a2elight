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

#ifndef __A2E_GUI_FILE_DIALOG_HPP__
#define __A2E_GUI_FILE_DIALOG_HPP__

#include "global.hpp"
#include "core/core.hpp"
#include "gui/gui_fwd.hpp"
#include "gui/objects/gui_object_event.hpp"

class A2E_API gui_file_dialog : public gui_object_event {
public:
	gui_file_dialog(const string& directory,
					const string file_extension = "", // any file/folder
					const string file_description = "", // -> "All Files and Folders"
					function<bool(const string&)> file_filter = [](const string&) { return false; });
	gui_file_dialog(const string& directory,
					const file_io::FILE_TYPE& file_type,
					function<bool(const string&)> file_filter = [](const string&) { return false; });
	virtual ~gui_file_dialog();
	
	virtual const string& get_file_name() const;
	virtual bool is_open() const;
	
protected:
	gui_window* dialog_wnd { nullptr };
	gui_button* last_button { nullptr };
	gui_button* next_button { nullptr };
	gui_button* cancel_button { nullptr };
	gui_button* action_button { nullptr };
	gui_input_box* file_name_input { nullptr };
	gui_pop_up_button* extensions_list { nullptr };
	gui_list_box* directory_entries { nullptr };
	gui_list_box* volumes_list { nullptr };
	
	enum class EXT_FILTER {
		EXTENSION_STRING,
		FILE_TYPE
	} ext_filter;
	string file_extension { "" };
	string file_description { "" };
	file_io::FILE_TYPE file_type { file_io::FILE_TYPE::NONE };
	function<bool(const string&)> file_filter;
	
	vector<string> dir_history;
	ssize_t dir_history_pos { -1 };
	
	//! the file name that was active when the action button was pressed
	string file_name { "" };
	
	typedef map<string, file_io::FILE_TYPE> directory_entries_container;
	virtual directory_entries_container get_directory_entries(const string& directory);
	virtual void open(const string directory, const bool write_history = true); //!< args must be copies!
	virtual void close();
	virtual void set_file_name();
	
};

#endif
