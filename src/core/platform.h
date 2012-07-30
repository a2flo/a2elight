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

/*! @brief platform header
 *  @author flo
 *
 *  do/include platform specific stuff here
 */

///////////////////////////////////////////////////////////////
// Windows
#if (defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(__WINDOWS__) || defined(MINGW)) && !defined(CYGWIN)

#include <windows.h>
#include <winnt.h>
#include <io.h>
#include <direct.h>

// defines
#if !defined(__WINDOWS__)
#define __WINDOWS__ 1
#endif

#if !defined(strtof)
#define strtof(arg1, arg2) ((float)strtod(arg1, arg2))
#endif

#if !defined(__func__)
#define __func__ __FUNCTION__
#endif

#if !defined(__FLT_MAX__)
#define __FLT_MAX__ FLT_MAX
#endif

#if !defined(SIZE_T_MAX)
#if defined(MAXSIZE_T)
#define SIZE_T_MAX MAXSIZE_T
#else
#define SIZE_T_MAX (~((size_t)0))
#endif
#endif

#undef getcwd
#define getcwd _getcwd

#pragma warning(disable: 4251)
#pragma warning(disable: 4290) // unnecessary exception throw warning
#pragma warning(disable: 4503) // srsly microsoft? this ain't the '80s ...

// Mac OS X
#elif defined(__APPLE__)
#include <dirent.h>
#define A2E_API

// everything else (Linux, *BSD, ...)
#else
#define A2E_API
#include <dirent.h>

#if !defined(SIZE_T_MAX)
#define SIZE_T_MAX (~((size_t)0))
#endif

#endif // Windows


// general includes
#if defined(__APPLE__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_cpuinfo.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2_image/SDL_image.h>
#if !defined(A2E_IOS)
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#else
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#elif defined(__WINDOWS__) && !defined(WIN_UNIXENV)
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_cpuinfo.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_image.h>
#include <GL/gl3.h>
#include <GL/wglext.h>

#elif defined(MINGW)
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_cpuinfo.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_image.h>
#define GL3_PROTOTYPES
#include <GL/gl3.h>

#else
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_cpuinfo.h>
#include <SDL_image.h>
#include <SDL_platform.h>
#include <SDL_syswm.h>
#include <GL/gl3.h>
#if !defined(WIN_UNIXENV)
#include <GL/glx.h>
#include <GL/glxext.h>
#endif
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#endif

#if !defined(A2E_IOS)
#define A2E_DEFAULT_FRAMEBUFFER 0
#else
#define A2E_DEFAULT_FRAMEBUFFER 1
#endif

// c++ headers
#include "core/cpp_headers.h"

// a2e logger
#include "core/logger.h"

#if !defined(__has_feature)
#define __has_feature(x) 0
#endif

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
#endif
