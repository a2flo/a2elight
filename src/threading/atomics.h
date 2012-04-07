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

#ifndef __A2E_ATOMICS_H__
#define __A2E_ATOMICS_H__

#include "core/cpp_headers.h"
using namespace std;

// TODO: use C++11 atomics when supported by clang/gcc

////////////////////////////////////////////////////////////////////////////////
// SDL 2.0+ atomic functions
#include <SDL2/SDL_atomic.h>

// check if atomics and sdl 2.0 are available
#if !defined(_SDL_atomic_h_ ) || (SDL_MAJOR_VERSION == 1)
#error "A2E requires SDL 2.0 with support for atomics"
#endif

#define AtomicFetchThenIncrement(a) SDL_AtomicAdd(a, 1)
#define AtomicFetchThenDecrement(a) SDL_AtomicAdd(a, -1)
#define AtomicFetchThenAdd(a, val) SDL_AtomicAdd(a, val)
#define AtomicGet(a) SDL_AtomicGet(a)
#define AtomicClear(a) SDL_AtomicSet(a, 0)
#define AtomicTestThenSet(a) SDL_AtomicCAS(a, 0, 1)
#define AtomicCAS(a, old, new) SDL_AtomicCAS(a, old, new)
#define AtomicSet(a, val) SDL_AtomicSet(a, val)
#define AtomicOR(a, val) __sync_fetch_and_or(&(a)->value, val)
#define AtomicAND(a, val) __sync_fetch_and_and(&(a)->value, val)
#define AtomicNAND(a, val) __sync_fetch_and_nand(&(a)->value, val)
#define AtomicXOR(a, val) __sync_fetch_and_xor(&(a)->value, val)
#define atomic_t SDL_atomic_t

#endif
