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

#include "unicode.h"

/*! creates a unicode object
 */
unicode::unicode() {
}

/*! unicode destructor
 */
unicode::~unicode() {
}

size_t unicode::get_utf8_strlen(const unsigned char* cstr) {
	size_t i;
	size_t len = 0;
	size_t size = strlen((const char*)cstr);
	bool invalid_size = false, invalid_byte = false;
	for(i = 0; i < size; i++) {
		// invalid start byte according to rfc standard
		if(cstr[i] >= 0xF5 || cstr[i] == 0xC0 || cstr[i] == 0xC1) {
			invalid_byte = true;
			break;
		}
		// oldschool ascii
		else if(cstr[i] <= 0x7F) {
			len++;
		}
		// 2-byte char
		else if((cstr[i] & 0xE0) == 0xC0) {
			if(i+1 < size) {
				if((cstr[i+1] & 0xC0) == 0x80) {
					len++;
					i++;
				}
				else {
					invalid_byte = true;
					break;
				}
			}
			else {
				invalid_size = true;
				break;
			}
		}
		// 3-byte char
		else if((cstr[i] & 0xF0) == 0xE0) {
			if(i+2 < size) {
				if((cstr[i+1] & 0xC0) == 0x80 && (cstr[i+2] & 0xC0) == 0x80) {
					len++;
					i+=2;
				}
				else {
					invalid_byte = true;
					break;
				}
			}
			else {
				invalid_size = true;
				break;
			}
		}
		// 4-byte char
		else if((cstr[i] & 0xF8) == 0xF0) {
			if(i+3 < size) {
				if((cstr[i+1] & 0xC0) == 0x80 && (cstr[i+2] & 0xC0) == 0x80 && (cstr[i+3] & 0xC0) == 0x80) {
					len++;
					i+=3;
				}
				else {
					invalid_byte = true;
					break;
				}
			}
			else {
				invalid_size = true;
				break;
			}
		}
		// another invalid start byte (possible?)
		else {
			invalid_byte = true;
			break;
		}
	}
	
	if(invalid_size) {
		a2e_error("utf-8 string has an invalid size of %u (len: %u / pos: %u)!", size, len, i);
		return 0;
	}
	
	if(invalid_byte) {
		a2e_error("utf-8 string has an invalid byte (len: %u / pos: %u)!", len, i);
		return 0;
	}
	
	return len;
}

void unicode::create_utf8_index_table(const unsigned char* cstr, map<size_t, size_t>& index_table) {
	size_t len = get_utf8_strlen(cstr);
	if(len == 0) return;
	
	index_table.clear();
	
	// this doesn't check for utf-8 or string validity, since this was already done in get_utf8_strlen()
	size_t cur_pos = 0;
	size_t size = strlen((const char*)cstr);
	for(size_t i = 0; i < size; i++) {
		index_table[cur_pos] = i;
		
		if((cstr[i] & 0xE0) == 0xC0) i++;
		else if((cstr[i] & 0xF0) == 0xE0) i+=2;
		else if((cstr[i] & 0xF8) == 0xF0) i+=3;
		
		cur_pos++;
	}
	index_table[cur_pos] = size; // behind last string byte, actually position of '\0'
}

string unicode::get_utf8_substr(const unsigned char* cstr, size_t begin, size_t end, map<size_t, size_t>& index_table) {
	size_t len = get_utf8_strlen(cstr);
	if(len == 0 || begin == end) return string("");
	if(end > len) {
		a2e_error("invalid end parameter (%u), string is only %u characters long!", end, len);
		return string("");
	}
	if(begin > end) {
		a2e_error("invalid begin parameter (%u), begin is greater than end (%u)!", begin, end);
		return string("");
	}
	
	return string((const char*)cstr).substr(index_table[begin], index_table[end] - index_table[begin]);
}

void unicode::create_charmap(const unsigned char* cstr, map<size_t, unsigned char*>& charmap, map<size_t, size_t>& index_table) {
	size_t len = get_utf8_strlen(cstr);
	if(len == 0) return;
	
	size_t cur_pos = 0;
	size_t size = strlen((const char*)cstr);
	for(size_t i = 0; i < size; i++) {
		if(!(cstr[i] & 0x80)) {
			charmap[cur_pos] = new unsigned char[2];
			charmap[cur_pos][0] = cstr[i];
			charmap[cur_pos][1] = 0;
			cur_pos++;
		}
		else if((cstr[i] & 0xE0) == 0xC0) {
			charmap[cur_pos] = new unsigned char[3];
			memcpy(charmap[cur_pos], &cstr[i], 2);
			charmap[cur_pos][2] = 0;
			i++;
			cur_pos++;
		}
		else if((cstr[i] & 0xF0) == 0xE0) {
			charmap[cur_pos] = new unsigned char[4];
			memcpy(charmap[cur_pos], &cstr[i], 3);
			charmap[cur_pos][3] = 0;
			i+=2;
			cur_pos++;
		}
		else if((cstr[i] & 0xF8) == 0xF0) {
			charmap[cur_pos] = new unsigned char[5];
			memcpy(charmap[cur_pos], &cstr[i], 4);
			charmap[cur_pos][4] = 0;
			i+=3;
			cur_pos++;
		}
		else break; // not possible actually ... validity already checked
	}
}

void unicode::utf8_str_insert(string& str, const unsigned char* insert_str, size_t insert_pos, map<size_t, size_t>& index_table) {
	const unsigned char* cstr = (const unsigned char*)str.c_str();
	size_t len = get_utf8_strlen(cstr);
	if(insert_pos > len) {
		a2e_error("invalid insert position (%u), string is only %u characters long!", insert_pos, len);
		return;
	}
	
	str.insert(index_table[insert_pos], (const char*)insert_str);
}

void unicode::utf8_str_erase(string& str, size_t begin, size_t end, map<size_t, size_t>& index_table) {
	const unsigned char* cstr = (const unsigned char*)str.c_str();
	size_t len = get_utf8_strlen(cstr);
	if(len == 0) return;
	if(end > len) {
		a2e_error("invalid end parameter (%u), string is only %u characters long!", end, len);
		return;
	}
	if(begin > end) {
		a2e_error("invalid begin parameter (%u), begin is greater than end (%u)!", begin, end);
		return;
	}
	
	str.erase(index_table[begin], index_table[end] - index_table[begin]);
}
