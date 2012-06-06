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

#ifndef __A2E_VERSION__
#define __A2E_VERSION__

// engine version and build/compiler info
#include "build_version.h"
#define A2E_MAJOR_VERSION "0"
#define A2E_MINOR_VERSION "2"
#define A2E_REVISION_VERSION "1"
#define A2E_DEV_STAGE_VERSION "b5"
#define A2E_BUILD_TIME __TIME__
#define A2E_BUILD_DATE __DATE__

#if defined(A2E_DEBUG) || defined(DEBUG)
#define A2E_DEBUG_STR " (debug)"
#else
#define A2E_DEBUG_STR ""
#endif

#if defined(_MSC_VER)
#define A2E_COMPILER "VC++ "+size_t2string(_MSC_VER)
#elif (defined(__GNUC__) && !defined(__llvm__) && !defined(__clang__))
#define A2E_COMPILER string("GCC ")+__VERSION__
#elif (defined(__GNUC__) && defined(__llvm__) && !defined(__clang__))
#define A2E_COMPILER string("LLVM-GCC ")+__VERSION__
#elif defined(__clang__)
#define A2E_COMPILER string("Clang ")+__clang_version__
#else
#define A2E_COMPILER "unknown compiler"
#endif

string _A2E_VERSION_TO_STR(const size_t& version);
string _A2E_VERSION_TO_STR(const size_t& version) {
	stringstream buffer; buffer << version; return buffer.str();
}

#define A2E_LIBCXX_PREFIX " and "
#if defined(_LIBCPP_VERSION)
#define A2E_LIBCXX A2E_LIBCXX_PREFIX+"libc++ "+_A2E_VERSION_TO_STR(_LIBCPP_VERSION)
#elif defined(__GLIBCXX__)
#define A2E_LIBCXX A2E_LIBCXX_PREFIX+"libstdc++ "+_A2E_VERSION_TO_STR(__GLIBCXX__)
#else
#define A2E_LIBCXX ""
#endif

#if !defined(A2E_IOS)
#define A2E_PLATFORM (sizeof(void*) == 4 ? "x86" : (sizeof(void*) == 8 ? "x64" : "unknown"))
#else
#define A2E_PLATFORM (sizeof(void*) == 4 ? "ARM" : (sizeof(void*) == 8 ? "ARM64" : "unknown"))
#endif

#define A2E_VERSION_STRING (string("A2E::light ")+A2E_PLATFORM+A2E_DEBUG_STR \
" v"+(A2E_MAJOR_VERSION)+"."+(A2E_MINOR_VERSION)+"."+(A2E_REVISION_VERSION)+(A2E_DEV_STAGE_VERSION)+"-"+size_t2string(A2E_BUILD_VERSION)+\
" ("+A2E_BUILD_DATE+" "+A2E_BUILD_TIME+") built with "+string(A2E_COMPILER+A2E_LIBCXX))

#define A2E_SOURCE_URL "http://www.albion2.org"


// compiler checks:
// msvc check
#if defined(_MSC_VER)
#if (_MSC_VER <= 1700)
#error "Sorry, but you need MSVC 12.0+ to compile A2E"
#endif

// clang check
#elif defined(__clang__)
#if !__has_feature(cxx_rvalue_references) || \
	!__has_feature(cxx_auto_type) || \
	!__has_feature(cxx_variadic_templates) || \
	!__has_feature(cxx_range_for) || \
	!__has_feature(cxx_lambdas) || \
	!__has_feature(cxx_generalized_initializers) || \
	!__has_feature(cxx_constexpr) || \
	!__has_feature(cxx_nonstatic_member_init)
#error "Sorry, but you need Clang with support for 'rvalue_references', 'auto_type', 'variadic_templates', 'range_for', 'lambdas', 'generalized_initializers', 'constexpr' and 'nonstatic_member_init' to compile A2E"
#endif

// gcc check
#elif defined(__GNUC__)
#if (__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
#error "Sorry, but you need GCC 4.7+ to compile A2E"
#endif

// just fall through ...
#else
#endif

#endif
