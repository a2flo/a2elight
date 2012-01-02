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


#include "logger.h"

#define A2E_LOG_FILENAME "log.txt"

fstream logger::log_file(A2E_LOG_FILENAME, fstream::out);
atomic_t logger::err_counter;
SDL_SpinLock logger::slock;
const config* logger::conf = NULL;

void logger::init() {
	logger::err_counter.value = 0;
	if(!log_file.is_open()) {
		cout << "LOG ERROR: couldn't open log file!" << endl;
	}
}

void logger::destroy() {
	log_file.close();
}

void logger::prepare_log(stringstream& buffer, const LOG_TYPE& type, const char* file, const char* func) {
	if(type != logger::LT_NONE) {
		buffer << logger::type_to_str(type);
		if(type == logger::LT_ERROR) buffer << " #" << AtomicFetchThenIncrement(&err_counter);
		buffer << ": ";
		// prettify file string (aka strip path)
		string file_str = file;
		size_t slash_pos = string::npos;
		if((slash_pos = file_str.rfind("/")) == string::npos) slash_pos = file_str.rfind("\\");
		file_str = (slash_pos != string::npos ? file_str.substr(slash_pos+1, file_str.size()-slash_pos-1) : file_str);
		buffer << file_str;
		buffer << ": " << func << "(): ";
	}
}

void logger::_log(stringstream& buffer, const char* str) {
	// this is the final log function
	while(*str) {
		if(*str == '%' && *(++str) != '%') {
			cout << "LOG ERROR: invalid log format, missing arguments!" << endl;
		}
		buffer << *str++;
	}
	buffer << endl;
	
	// finally: output
	SDL_AtomicLock(&slock);
	cout << buffer.str();
	log_file << buffer.str();
	log_file.flush();
	SDL_AtomicUnlock(&slock);
}

