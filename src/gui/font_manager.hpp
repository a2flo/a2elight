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

#ifndef __A2E_FONT_MANAGER_HPP__
#define __A2E_FONT_MANAGER_HPP__

#include "global.hpp"
#include <floor/threading/thread_base.hpp>

class rtt;
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_* FT_Library;
class a2e_font;

//! loads and caches fonts
class font_manager : public thread_base {
public:
	font_manager();
	virtual ~font_manager();
	FT_Library get_ft_library();
	
	a2e_font* add_font(const string& identifier, const string& filename);
	a2e_font* add_font_family(const string& identifier, vector<string>&& filenames);
	a2e_font* get_font(const string& identifier) const;
	bool remove_font(const string& identifier);
	
	virtual void run();

protected:
	rtt* r;

	// identifier -> font object
	unordered_map<string, a2e_font*> fonts;
	FT_Library ft_library;

};

#endif
