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

#ifndef __A2E_GL_TIMER_H__
#define __A2E_GL_TIMER_H__

#include "global.h"
#include "core/vector3.h"

/*! @class gl_timer
 *  @brief arb_timer_query wrapper + additional functionality
 */

class A2E_API gl_timer {
public:
	//
	gl_timer() = delete;
	~gl_timer() = delete;
	
	static constexpr size_t stored_frames = 16;
	struct frame_info {
		struct query_object {
			const string identifier;
			unsigned int query_ref;
			float3 color;
			GLuint64 time;
		};
		vector<query_object> queries;
		bool done = false;
		bool available = false;
	};
	static const frame_info* get_last_available_frame();
	
	static void init();
	static void destroy();
	static void state_check();
	
	static void start_frame();
	static void mark(const string& identifier);
	static void stop_frame();

protected:
	static array<frame_info, stored_frames> frames; // keep last 16 frames
	static size_t cur_frame;
	static bool enabled;
	
	// available gl query objects
	static array<unsigned int, stored_frames * 32> gl_queries;
	static vector<unsigned int> query_store;
	
};

#endif
