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

#ifndef __A2E_LOGGER_H__
#define __A2E_LOGGER_H__

#include "cpp_headers.h"
#include "threading/atomics.h"
using namespace std;

//! a2e logging functions, use appropriately
//! note that you don't actually have to use a specific character for %_ to print the
//! correct type (the ostream operator<< is used and the %_ character is ignored - except
//! for %x and %X which will print out an integer in hex format)
#define a2e_error(...) logger::log(logger::LOG_TYPE::ERROR_MSG, __FILE__, __func__, __VA_ARGS__)
#define a2e_debug(...) logger::log(logger::LOG_TYPE::DEBUG_MSG, __FILE__, __func__, __VA_ARGS__)
#define a2e_msg(...) logger::log(logger::LOG_TYPE::SIMPLE_MSG, __FILE__, __func__, __VA_ARGS__)
#define a2e_log(...) logger::log(logger::LOG_TYPE::NONE, __FILE__, __func__, __VA_ARGS__)

class A2E_API logger {
public:
	enum class LOG_TYPE {
		NONE,		//!< enum message with no prefix
		SIMPLE_MSG,	//!< enum simple message
		ERROR_MSG,	//!< enum error message
		DEBUG_MSG	//!< enum debug message
	};
	
	static void init();
	static void destroy();
	
	//
	static const char* type_to_str(const LOG_TYPE& type) {
		switch(type) {
			case LOG_TYPE::NONE: return "";
			case LOG_TYPE::SIMPLE_MSG: return "[ MSG ]";
			case LOG_TYPE::ERROR_MSG: return "[ERROR]";
			case LOG_TYPE::DEBUG_MSG: return "[DEBUG]";
		}
		assert(false && "invalid log type");
		return "UNKNOWN";
	}
	
	// log entry function, this will create a buffer and insert the log msgs start info (type, file name, ...) and
	// finally call the internal log function (that does the actual logging)
	template<typename... Args> static void log(const LOG_TYPE type, const char* file, const char* func, const char* str, Args&&... args) {
		stringstream buffer;
		prepare_log(buffer, type, file, func);
		_log(buffer, str, std::forward<Args>(args)...);
	}
	
protected:
	logger(const logger& l) = delete;
	~logger() = delete;
	logger& operator=(const logger& l) = delete;
	
	static ofstream log_file;
	static atomic_t err_counter;
	static SDL_SpinLock slock;
	
	//
	static void prepare_log(stringstream& buffer, const LOG_TYPE& type, const char* file, const char* func);
	
	//! handles the log format
	//! only %x and %X are supported at the moment, in all other cases the standard ostream operator<< is used!
	template <typename T> static void handle_format(stringstream& buffer, const char& format, T value) {
		switch(format) {
			case 'x':
				buffer << "0x" << hex << value << dec;
				break;
			case 'X':
				buffer << "0x" << hex << uppercase << value << nouppercase << dec;
				break;
			default:
				buffer << value;
				break;
		}
	}
	
	// internal logging functions
	static void _log(stringstream& buffer, const char* str); // will be called in the end (when there are no more args)
	template<typename T, typename... Args> static void _log(stringstream& buffer, const char* str, T value, Args&&... args) {
		while(*str) {
			if(*str == '%' && *(++str) != '%') {
				handle_format(buffer, *str, value);
				_log(buffer, ++str, std::forward<Args>(args)...);
				return;
			}
			buffer << *str++;
		}
		cout << "LOG ERROR: unused extra arguments specified in: \"" << buffer.str() << "\"!" << endl;
	}
	
};

#endif
