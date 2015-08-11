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

#ifndef __A2E_VERSION__
#define __A2E_VERSION__

// engine version and build/compiler info
#include "build_version.hpp"
#include <floor/core/util.hpp>
#define A2E_MAJOR_VERSION "0"
#define A2E_MINOR_VERSION "3"
#define A2E_REVISION_VERSION "1"
#define A2E_DEV_STAGE_VERSION "d4"
#define A2E_BUILD_TIME __TIME__
#define A2E_BUILD_DATE __DATE__

#if defined(A2E_DEBUG) || defined(DEBUG)
#define A2E_DEBUG_STR " (debug)"
#else
#define A2E_DEBUG_STR ""
#endif

#if defined(_MSC_VER)
#define A2E_COMPILER "VC++ "+to_string(_MSC_VER)
#elif (defined(__GNUC__) && !defined(__llvm__) && !defined(__clang__))
#define A2E_COMPILER string("GCC ")+__VERSION__
#elif (defined(__GNUC__) && defined(__llvm__) && !defined(__clang__))
#define A2E_COMPILER string("LLVM-GCC ")+__VERSION__
#elif defined(__clang__)
#define A2E_COMPILER string("Clang ")+__clang_version__
#else
#define A2E_COMPILER "unknown compiler"
#endif

#define A2E_LIBCXX_PREFIX " and "
#if defined(_LIBCPP_VERSION)
#define A2E_LIBCXX A2E_LIBCXX_PREFIX+"libc++ "+to_string(_LIBCPP_VERSION)
#elif defined(__GLIBCXX__)
#define A2E_LIBCXX A2E_LIBCXX_PREFIX+"libstdc++ "+to_string(__GLIBCXX__)
#else
#define A2E_LIBCXX ""
#endif

#if !defined(FLOOR_IOS)
#define A2E_PLATFORM (sizeof(void*) == 4 ? "x86" : (sizeof(void*) == 8 ? "x64" : "unknown"))
#else
#define A2E_PLATFORM (sizeof(void*) == 4 ? "ARM32" : (sizeof(void*) == 8 ? "ARM64" : "unknown"))
#endif

#define A2E_VERSION_STRING (string("A2E::light ")+A2E_PLATFORM+A2E_DEBUG_STR \
" v"+(A2E_MAJOR_VERSION)+"."+(A2E_MINOR_VERSION)+"."+(A2E_REVISION_VERSION)+(A2E_DEV_STAGE_VERSION)+"-"+to_string(A2E_BUILD_VERSION)+\
" ("+A2E_BUILD_DATE+" "+A2E_BUILD_TIME+") built with "+string(A2E_COMPILER+A2E_LIBCXX))

#define A2E_SOURCE_URL "https://www.albion2.org"


// compiler checks:
// msvc check
#if defined(_MSC_VER)
#if (_MSC_VER <= 1800)
#error "Sorry, but you need MSVC 13.0+ (VS 2014+) to compile A2E"
#endif

// clang check
#elif defined(__clang__)
#if !defined(__clang_major__) || !defined(__clang_minor__) || (__clang_major__ < 3) || (__clang_major__ == 3 && __clang_minor__ < 5)
#error "Sorry, but you need Clang 3.5+ to compile A2E"
#endif

// gcc check
#elif defined(__GNUC__)
#if (__GNUC__ < 5)
#error "Sorry, but you need GCC 5.x+ to compile A2E"
#endif

// just fall through ...
#else
#endif

#endif
