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

#ifndef __UNICODE_H__
#define __UNICODE_H__

#include "global.h"

/*! @class unicode
 *  @brief unicode routines
 */

class A2E_API unicode {
public:
	unicode();
	~unicode();
	
	size_t get_utf8_strlen(const unsigned char* cstr);
	string get_utf8_substr(const unsigned char* cstr, size_t begin, size_t end, map<size_t, size_t>& index_table);
	void create_utf8_index_table(const unsigned char* cstr, map<size_t, size_t>& index_table);
	void create_charmap(const unsigned char* cstr, map<size_t, unsigned char*>& charmap, map<size_t, size_t>& index_table);
	void utf8_str_insert(string& str, const unsigned char* insert_str, size_t insert_pos, map<size_t, size_t>& index_table);
	void utf8_str_erase(string& str, size_t begin, size_t end, map<size_t, size_t>& index_table);

protected:
	wstring wc_buffer;
	string c_buffer;
	
};

#endif
