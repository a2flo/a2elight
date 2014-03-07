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

#include "gui/compound/gui_file_dialog.hpp"
#include "gui/gui.hpp"
#include "gui/objects/gui_window.hpp"
#include "gui/objects/gui_button.hpp"
#include "gui/objects/gui_text.hpp"
#include "gui/objects/gui_pop_up_button.hpp"
#include "gui/objects/gui_list_box.hpp"
#include "gui/objects/gui_input_box.hpp"
#include "threading/task.hpp"

#if defined(__APPLE__)
#if !defined(FLOOR_IOS)
#include "osx/osx_helper.hpp"
#else
#include "ios/ios_helper.hpp"
#endif
#endif

struct file_extension_info {
	const char* description;
	const vector<const char*> extensions;
};
static const unordered_map<file_io::FILE_TYPE, file_extension_info> known_file_extensions {
	{ file_io::FILE_TYPE::NONE, { "All Files and Folders", { "" } } },
	{ file_io::FILE_TYPE::DIR, { "All Folders", { "" } } },
	{ file_io::FILE_TYPE::TEXT, { "Text Files", { "txt" } } },
	{ file_io::FILE_TYPE::IMAGE, { "Image Files", { "png", "jpeg", "jpg", "gif", "bmp", "tif", "tiff", "tga" } } },
	{ file_io::FILE_TYPE::XML, { "XML Files", { "xml" } } },
	{ file_io::FILE_TYPE::OPENCL, { "OpenCL Files", { "cl", "clh", "h" } } },
	{ file_io::FILE_TYPE::A2E_MODEL, { "A2E Model Files", { "a2m" } } },
	{ file_io::FILE_TYPE::A2E_ANIMATION, { "A2E Animation Files", { "a2a" } } },
	{ file_io::FILE_TYPE::A2E_MATERIAL, { "A2E Material Files", { "a2mat" } } },
	{ file_io::FILE_TYPE::A2E_UI, { "A2E UI Files", { "a2eui" } } },
};

gui_file_dialog::gui_file_dialog(const string& directory floor_unused,
								 const string file_extension_,
								 const string file_description_,
								 function<bool(const string&)> file_filter_) :
gui_object_event(),
// NOTE: if (file_extension_ == ""), then file_type is already initialized with FILE_TYPE::NONE
ext_filter(file_extension_ == "" ? EXT_FILTER::FILE_TYPE : EXT_FILTER::EXTENSION_STRING),
file_extension(file_extension_), file_description(file_description_), file_filter(file_filter_) {
}

gui_file_dialog::gui_file_dialog(const string& directory floor_unused,
								 const file_io::FILE_TYPE& file_type_,
								 function<bool(const string&)> file_filter_) :
gui_object_event(),
ext_filter(EXT_FILTER::FILE_TYPE), file_type(file_type_), file_filter(file_filter_) {
}

gui_file_dialog::~gui_file_dialog() {
	close();
}

const string& gui_file_dialog::get_file_name() const {
	return file_name;
}

gui_file_dialog::directory_entries_container gui_file_dialog::get_directory_entries(const string& directory) {
	directory_entries_container file_list;
	if(ext_filter == EXT_FILTER::EXTENSION_STRING) {
		file_list = core::get_file_list(directory, file_extension, true);
		core::erase_if(file_list, [this](const typename decltype(file_list)::iterator& iter) {
			return file_filter(iter->first);
		});
	}
	else { // EXT_FILTER::FILE_TYPE
		const auto ext_iter = known_file_extensions.find(file_type);
		if(ext_iter == known_file_extensions.end()) {
			log_error("unknown file type: %u", file_type);
			return directory_entries_container {};
		}
		
		if(file_type == file_io::FILE_TYPE::NONE ||
		   file_type == file_io::FILE_TYPE::DIR) {
			file_list = core::get_file_list(directory, "");
			if(file_type == file_io::FILE_TYPE::DIR) {
				// filter out files
				core::erase_if(file_list, [this](const typename decltype(file_list)::iterator& iter) {
					return (iter->second != file_type);
				});
			}
		}
		else {
			// iterate over all extension and add all matched files to file_list
			for(const auto& file_ext : ext_iter->second.extensions) {
				const auto file_list_ext = core::get_file_list(directory, file_ext);
				file_list.insert(file_list_ext.begin(), file_list_ext.end());
			}
			
			// add directories
			auto dir_list = core::get_file_list(directory, "");
			core::erase_if(dir_list, [this](const typename decltype(file_list)::iterator& iter) {
				return (iter->second != file_io::FILE_TYPE::DIR);
			});
			file_list.insert(dir_list.begin(), dir_list.end());
		}
		
		// apply file filter
		core::erase_if(file_list, [this](const typename decltype(file_list)::iterator& iter) {
			return file_filter(iter->first);
		});
	}
	return file_list;
}

void gui_file_dialog::open(const string directory, const bool write_history) {
	// handle directory history
	if(write_history) {
		// delete old/front history
		if(dir_history_pos >= 0 && (size_t)dir_history_pos != dir_history.size() - 1) {
			dir_history.erase(dir_history.begin() + dir_history_pos + 1, dir_history.end());
		}
		dir_history.emplace_back(directory);
		dir_history_pos = -1;
	}
	
	//
	const directory_entries_container dir_entries { get_directory_entries(directory) };
	
	// create the ui
	if(dialog_wnd == nullptr) {
		gui* ui = engine::get_gui();
		dialog_wnd = ui->add<gui_window>(float2(1.0f, 1.0f), float2(0.0f, 0.0f));
		dialog_wnd->set_background_color(float4 { 1.0f });
		
		static constexpr float elem_height { 0.03f };
		static constexpr float margin { 0.003f };
		static constexpr float2 x_offsets { margin, 0.2f };
		static constexpr float3 y_offsets {
			margin, // 0 + margin
			elem_height + margin * 2.0f, // 1st + above/below margin
			1.0f - (elem_height + margin), // reverse: substract 3rd height + margin from total
		};
		static constexpr float2 x_sizes { 0.2f - margin * 2.0f, 0.8f - margin };
		static constexpr float3 y_sizes { elem_height, 1.0f - elem_height * 2.0f - margin * 4.0f, elem_height };
		
		directory_entries = ui->add<gui_list_box>(float2(x_sizes.y, y_sizes.y),
												  float2(x_offsets.y, y_offsets.y));
		volumes_list = ui->add<gui_list_box>(float2(x_sizes.x, y_sizes.y),
											 float2(x_offsets.x, y_offsets.y));
		dialog_wnd->add_child(directory_entries);
		dialog_wnd->add_child(volumes_list);
		
		//
		file_name_input = ui->add<gui_input_box>(float2(x_sizes.y, y_sizes.x),
												 float2(x_offsets.y, y_offsets.x));
		static constexpr float button_width = x_sizes.x * 0.5f - margin * 0.5f;
		last_button = ui->add<gui_button>(float2(button_width, y_sizes.x),
										  float2(x_offsets.x, y_offsets.x));
		next_button = ui->add<gui_button>(float2(button_width, y_sizes.x),
										  float2(x_offsets.x + button_width + margin, y_offsets.x));
		cancel_button = ui->add<gui_button>(float2(button_width, y_sizes.z),
											float2(1.0f - x_offsets.y + margin, y_offsets.z));
		action_button = ui->add<gui_button>(float2(button_width, y_sizes.z),
											float2(1.0f - x_offsets.y + margin * 2.0f + button_width, y_offsets.z));
		last_button->set_label("<");
		next_button->set_label(">");
		cancel_button->set_label("Cancel");
		action_button->set_label("<...>");
		dialog_wnd->add_child(file_name_input);
		dialog_wnd->add_child(last_button);
		dialog_wnd->add_child(next_button);
		dialog_wnd->add_child(cancel_button);
		dialog_wnd->add_child(action_button);
		
		//
		extensions_list = ui->add<gui_pop_up_button>(float2(x_sizes.y, y_sizes.z),
													 float2(x_offsets.x, y_offsets.z));
		extensions_list->add_item(to_string((unsigned int)file_io::FILE_TYPE::NONE), "All Files and Folders");
		if(ext_filter == EXT_FILTER::FILE_TYPE && file_type != file_io::FILE_TYPE::NONE) {
			const auto ext_iter = known_file_extensions.find(file_type);
			if(ext_iter != known_file_extensions.end()) {
				const string type_str { to_string((unsigned int)file_type) };
				extensions_list->add_item(type_str, ext_iter->second.description);
				extensions_list->set_selected_item(type_str, false, false);
			}
		}
		else if(ext_filter == EXT_FILTER::EXTENSION_STRING) {
			extensions_list->add_item(":" + file_extension, file_description);
			extensions_list->set_selected_item(":" + file_extension, false, false);
		}
		dialog_wnd->add_child(extensions_list);
		
		// event handling
		last_button->add_handler([this](GUI_EVENT, gui_object&) {
			floor::acquire_context();
			if(dir_history_pos == -1) {
				dir_history_pos = dir_history.size() - 1;
			}
			--dir_history_pos;
			if(dir_history_pos < 0) dir_history_pos = 0;
			open(dir_history[dir_history_pos], false);
			floor::release_context();
		}, GUI_EVENT::BUTTON_PRESS);
		
		next_button->add_handler([this](GUI_EVENT, gui_object&) {
			floor::acquire_context();
			if(dir_history_pos != -1) {
				++dir_history_pos;
				const ssize_t max_history = dir_history.size() - 1;
				if(dir_history_pos > max_history) dir_history_pos = max_history;
				open(dir_history[dir_history_pos], false);
			}
			floor::release_context();
		}, GUI_EVENT::BUTTON_PRESS);
		
		cancel_button->add_handler([this](GUI_EVENT, gui_object&) {
			floor::acquire_context();
			close();
			floor::release_context();
		}, GUI_EVENT::BUTTON_PRESS);
		
		extensions_list->add_handler([this](GUI_EVENT, gui_object&) {
			floor::acquire_context();
			const auto& ext_id = extensions_list->get_selected_item()->first;
			if(ext_id[0] != ':') {
				file_type = (file_io::FILE_TYPE)stoul(ext_id);
				ext_filter = EXT_FILTER::FILE_TYPE;
			}
			else {
				file_extension = ext_id.substr(1);
				ext_filter = EXT_FILTER::EXTENSION_STRING;
			}
			open(dir_history_pos == -1 ? dir_history.back() : dir_history[dir_history_pos], false);
			floor::release_context();
		}, GUI_EVENT::POP_UP_BUTTON_SELECT);
		
		directory_entries->add_handler([this](GUI_EVENT, gui_object&) {
			// check if the location in the directory list is a file
			floor::acquire_context();
			const auto& location = directory_entries->get_selected_item()->first;
			if(location.back() != '/') {
				file_name_input->set_input(directory_entries->get_selected_item()->second);
			}
			floor::release_context();
		}, GUI_EVENT::LIST_BOX_SELECT);
		directory_entries->add_handler([this](GUI_EVENT, gui_object&) {
			// check if the location in the directory list is a directory
			floor::acquire_context();
			const auto& location = directory_entries->get_selected_item()->first;
			if(location.back() == '/' || location == "../") {
				open(location);
			}
			floor::release_context();
		}, GUI_EVENT::LIST_BOX_SELECT_EXECUTE);
		
		volumes_list->add_handler([this](GUI_EVENT, gui_object&) {
			floor::acquire_context();
			open(volumes_list->get_selected_item()->first);
			floor::release_context();
		}, GUI_EVENT::LIST_BOX_SELECT);
	}
	
	// add dir entries
	directory_entries->clear();
	bool has_upper_dir = (directory != "/");
	string upper_dir = "/";
	if(has_upper_dir) {
		const auto upper_dir_pos = directory.rfind('/', directory.size() > 2 ? directory.size() - 2 : 0);
		has_upper_dir = (upper_dir_pos != string::npos);
		if(has_upper_dir) {
			upper_dir = directory.substr(0, upper_dir_pos + 1);
		}
	}
	for(const auto& entry : dir_entries) {
		if(entry.first == ".") continue;
		if(entry.first == ".." && !has_upper_dir) continue;
		const string entry_str { entry.first + (entry.second == file_io::FILE_TYPE::DIR ? "/" : "") };
		directory_entries->add_item(entry.first == ".." ? upper_dir : directory + entry_str, entry_str);
	}
	if(directory_entries->get_selected_item() != nullptr) {
		file_name_input->set_input(directory_entries->get_selected_item()->second);
	}
	
	// add volumes
	volumes_list->clear();
#if defined(__WINDOWS__)
	// TODO: windows volume + user dir list
	volumes_list->add_item("/c/", "C:/");
#else // works under Linux, OS X and FreeBSD
	
	// host/root
#if defined(__APPLE__)
	volumes_list->add_item("/",
#if !defined(FLOOR_IOS)
						   osx_helper::get_computer_name()
#else
						   ios_helper::get_computer_name()
#endif
						   );
#else
#if defined(HOST_NAME_MAX)
#define A2E_HOST_NAME_SIZE (HOST_NAME_MAX + 1)
#elif defined(_POSIX_HOST_NAME_MAX)
#define A2E_HOST_NAME_SIZE (_POSIX_HOST_NAME_MAX + 1)
#else
#warning "neither HOST_NAME_MAX nor _POSIX_HOST_NAME_MAX is defined"
#define A2E_HOST_NAME_SIZE (1024)
#endif
	char hostname[A2E_HOST_NAME_SIZE];
	memset(hostname, 0, A2E_HOST_NAME_SIZE);
	gethostname(hostname, A2E_HOST_NAME_SIZE - 1);
	volumes_list->add_item("/", hostname);
#endif
	
	struct passwd* user_info = getpwnam(getenv("USER"));
	string user = user_info->pw_name + string("’s Home");
	user[0] = (char)toupper(user[0]); // make first letter uppercase
	string user_dir = user_info->pw_dir;
	endpwent();
	volumes_list->add_item(user_dir + "/", user);
	
	static constexpr const char* volume_dir = (
#if defined(__APPLE__)
											   "/Volumes"
#else
											   "/mnt"
#endif
											   );
	
	struct dirent** volumelist = nullptr;
	int n = scandir(volume_dir, &volumelist, 0, alphasort);
	for(int i = 1; i < n; i++) {
		if(volumelist[i]->d_type == DT_DIR && strcmp("..", volumelist[i]->d_name) != 0) {
			volumes_list->add_item(string(volume_dir) + "/" + string(volumelist[i]->d_name) + "/",
								   volumelist[i]->d_name);
		}
	}
	
	if(volumelist != nullptr) {
		for(int i = 0; i < n; i++) {
			free(volumelist[i]);
		}
		free(volumelist);
	}
#endif
	
	// if the current directory matches a volume directory, then select it
	if(volumes_list->has_item(directory)) {
		volumes_list->set_selected_item(directory, false, false);
	}
}

void gui_file_dialog::close() {
	if(dialog_wnd == nullptr) return;
	engine::get_gui()->remove(dialog_wnd);
	dialog_wnd = nullptr;
}

bool gui_file_dialog::is_open() const {
	return (dialog_wnd != nullptr);
}

void gui_file_dialog::set_file_name() {
	string input = file_name_input->get_input();
	if(input.empty()) {
		if(directory_entries->get_selected_item() == nullptr) {
			file_name = "";
			return;
		}
		input = directory_entries->get_selected_item()->second;
	}
	file_name = (dir_history_pos == -1 ? dir_history.back() : dir_history[dir_history_pos]) + input;
}
