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

#ifndef __TYPE_LIST_H__
#define __TYPE_LIST_H__

using namespace std;

class null_type {};
struct empty_type {};

// define typelists
template <typename... Args> struct type_list; // necessary for the gnu crap compiler (might be removed at a later point ...)
template <typename T, typename... Args> struct type_list<T, Args...> {
	typedef T head;
	typedef type_list<Args...> tail;
};
template <typename T> struct type_list<T> {
	typedef T head;
	typedef null_type tail;
};
template <> struct type_list<> {
	typedef null_type head;
	typedef null_type tail;
};

// typelist namespace and functions
namespace tl {
	// typelist length definitions
	template <class tlist> struct length;
	template <> struct length<null_type> {
		enum { value = 0 };
	};
	
	template <class T> struct length<type_list<T>> {
		enum { value = 1 };
	};
	
	template <class T, class... U> struct length<type_list<T, U...>> {
		enum { value = 1 + length<type_list<U...>>::value };
	};
	
	// typelist indexed access (strict)
	template <class tlist, size_t index> struct type_at;
	template <class head> struct type_at<type_list<head>, 0> {
		typedef head result;
	};
	template <class head, class... tail> struct type_at<type_list<head, tail...>, 0> {
		typedef head result;
	};
	template <class head, class... tail, size_t i> struct type_at<type_list<head, tail...>, i> {
		typedef typename type_at<type_list<tail...>, i - 1>::result result;
	};
	
	// typelist indexed access (non-strict)
	template <class tlist, size_t index, typename default_type = null_type> struct type_at_non_strict {
		typedef default_type result;
	};
	
	template <class head, typename default_type> struct type_at_non_strict<type_list<head>, 0, default_type> {
		typedef head result;
	};
	
	template <class head, class... tail, typename default_type> struct type_at_non_strict<type_list<head, tail...>, 0, default_type> {
		typedef head result;
	};
	
	template <class head, class... tail, size_t i, typename default_type> struct type_at_non_strict<type_list<head, tail...>, i, default_type> {
		typedef typename type_at_non_strict<type_list<tail...>, i - 1, default_type>::result result;
	};
}

#endif
