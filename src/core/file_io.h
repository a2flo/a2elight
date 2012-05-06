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

#ifndef __FILE_IO_H__
#define __FILE_IO_H__

#include "global.h"

#include "gui/unicode.h"

/*! @class file_io
 *  @brief file input/output
 */

class A2E_API file_io {
public:
	enum FIO_OPEN_TYPE {
		OT_READ,
		OT_READWRITE,
		OT_WRITE,
		OT_READ_BINARY,
		OT_READWRITE_BINARY,
		OT_WRITE_BINARY,
		OT_APPEND,
		OT_APPEND_BINARY,
		OT_APPEND_READ,
		OT_APPEND_READ_BINARY
	};
	
	file_io();
	file_io(const string& filename, FIO_OPEN_TYPE open_type = OT_READWRITE_BINARY);
	~file_io();
	
	enum FILE_TYPE {
		FT_NONE,
		FT_DIR,
		FT_ALL,
		FT_IMAGE,
		FT_A2E_MODEL,
		FT_A2E_ANIMATION,
		FT_A2E_MATERIAL,
		FT_A2E_MAP,
		FT_A2E_UI,
		FT_A2E_LIST,
		FT_A2E_SHADER,
		FT_XML,
		FT_TEXT,
		FT_OPENCL
	};

	bool open(const string& filename, FIO_OPEN_TYPE open_type);
	bool file_to_buffer(const string& filename, stringstream& buffer);
	void close();
	uint64_t get_filesize();
	fstream* get_filestream();
	
	// file input:
	void read_file(stringstream* buffer);
	void get_line(char* finput, unsigned int length);
	void get_block(char* data, size_t size);
	void get_terminated_block(string& str, const char terminator);
	char get_char();
	unsigned short int get_usint();
	unsigned int get_uint();
	float get_float();
	void seek(size_t offset);
	streampos get_current_offset();
	
	// file output:
	void write_block(const char* data, size_t size, bool check_size = false);
	void write_terminated_block(const string& str, const char terminator);
	void write_char(const unsigned char& ch);
	void write_uint(const unsigned int& ui);
	void write_float(const float& f);


	//
	bool is_file(const char* filename);
	bool eof() const;
	bool good() const;
	bool fail() const;
	bool bad() const;
	bool is_open() const;

protected:
	fstream filestream;

	bool check_open();

};

#endif
