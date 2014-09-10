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

#ifndef __A2E_GUI_FILE_SAVE_DIALOG_HPP__
#define __A2E_GUI_FILE_SAVE_DIALOG_HPP__

#include "gui/compound/gui_file_dialog.hpp"

class gui_file_save_dialog : public gui_file_dialog {
public:
	gui_file_save_dialog(const string& directory,
						 const string file_extension = "", // any file/folder
						 const string file_description = "", // -> "All Files and Folders"
						 function<bool(const string&)> file_filter = [](const string&) { return false; });
	gui_file_save_dialog(const string& directory,
						 const file_io::FILE_TYPE& file_type,
						 function<bool(const string&)> file_filter = [](const string&) { return false; });
	virtual ~gui_file_save_dialog();
	
protected:
	virtual void open(const string directory, const bool write_history = true) override;
	
};

#endif
