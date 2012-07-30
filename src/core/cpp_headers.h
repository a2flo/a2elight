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

#ifndef __A2E_CPP_HEADERS_H__
#define __A2E_CPP_HEADERS_H__

// on windows exports/imports: apparently these have to be treated separately,
// always use dllexport for a2e/c++ stuff and depending on compiling or using
// a2e, use dllexport or dllimport for all opengl functions
#if defined(A2E_EXPORTS)
#pragma warning(disable: 4251)
#define A2E_API __declspec(dllexport)
#define OGL_API __declspec(dllexport)
#elif defined(A2E_IMPORTS)
#pragma warning(disable: 4251)
#define A2E_API __declspec(dllexport)
#define OGL_API __declspec(dllimport)
#else
#define A2E_API
#define OGL_API
#endif // A2E_API_EXPORT

#if defined(__WINDOWS__) || defined(MINGW)
#include <windows.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <functional>
#include <vector>
#include <array>
#include <list>
#include <deque>
#include <queue>
#include <stack>
#include <map>
#include <unordered_map>
#include <set>
#include <limits>
#include <typeinfo>
#include <locale>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <ctime>
#include <cstring>
#include <cmath>
#include <cassert>

#if defined(CYGWIN)
#include <sys/wait.h>
#endif

using namespace std;

// don't use constexpr constructors with gcc
#if defined(__clang__)
#define a2e_constexpr constexpr
#else
#define a2e_constexpr
#endif

//
#define a2e_unused __attribute__((unused))

// we don't need these
#undef min
#undef max

#endif
