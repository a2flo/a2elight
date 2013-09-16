/*
 *  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2013 Florian Ziesche
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

#ifndef __A2E_GLOBAL_HPP__
#define __A2E_GLOBAL_HPP__

#include "core/platform.hpp"

// general includes
#if defined(__APPLE__)
#include <SDL2_image/SDL_image.h>
#if !defined(A2E_IOS)
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#else
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#elif defined(__WINDOWS__) && !defined(WIN_UNIXENV)
#include <SDL2/SDL_image.h>
#include <GL/gl3.h>
#include <GL/wglext.h>

#elif defined(MINGW)
#include <SDL2/SDL_image.h>
#define GL3_PROTOTYPES
#include <GL/gl3.h>

#else
#include <SDL_image.h>
#include <GL/gl3.h>
#if !defined(WIN_UNIXENV)
#include <GL/glx.h>
#include <GL/glxext.h>
#endif
#endif

#if !defined(A2E_IOS)
#define A2E_DEFAULT_FRAMEBUFFER 0
#else
#define A2E_DEFAULT_FRAMEBUFFER 1
#endif

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

// TODO: better location for this?
#if !defined(__A2E_DRAW_MODE_DEF__)
#define __A2E_DRAW_MODE_DEF__
enum class DRAW_MODE : unsigned int {
	NONE					= 0,
	
	GEOMETRY_PASS			= (1 << 0),
	MATERIAL_PASS			= (1 << 1),
	GEOMETRY_ALPHA_PASS		= (1 << 2),
	MATERIAL_ALPHA_PASS		= (1 << 3),
	GM_PASSES_MASK			= GEOMETRY_PASS | MATERIAL_PASS | GEOMETRY_ALPHA_PASS | MATERIAL_ALPHA_PASS,
	
	ENVIRONMENT_PASS		= (1 << 4), // note: this isn't used on it's own, but in combination with the four modes above
	ENV_GEOMETRY_PASS		= ENVIRONMENT_PASS | GEOMETRY_PASS,
	ENV_MATERIAL_PASS		= ENVIRONMENT_PASS | MATERIAL_PASS,
	ENV_GEOMETRY_ALPHA_PASS	= ENVIRONMENT_PASS | GEOMETRY_ALPHA_PASS,
	ENV_MATERIAL_ALPHA_PASS	= ENVIRONMENT_PASS | MATERIAL_ALPHA_PASS,
	ENV_GM_PASSES_MASK		= ENVIRONMENT_PASS | GM_PASSES_MASK
};
A2E_API DRAW_MODE operator|(const DRAW_MODE& e0, const DRAW_MODE& e1);
A2E_API DRAW_MODE& operator|=(DRAW_MODE& e0, const DRAW_MODE& e1);
A2E_API DRAW_MODE operator&(const DRAW_MODE& e0, const DRAW_MODE& e1);
A2E_API DRAW_MODE& operator&=(DRAW_MODE& e0, const DRAW_MODE& e1);
#endif

#endif
