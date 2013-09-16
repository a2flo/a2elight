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

#include "gl_timer.h"
#include "rendering/extensions.h"
#include "core/core.h"

constexpr size_t gl_timer::stored_frames;
array<gl_timer::frame_info, gl_timer::stored_frames> gl_timer::frames;
size_t gl_timer::cur_frame = 0;
bool gl_timer::enabled = false;

array<unsigned int, gl_timer::stored_frames * 32> gl_timer::gl_queries;
vector<unsigned int> gl_timer::query_store;

void gl_timer::init() {
	glGenQueries((GLsizei)gl_queries.size(), &gl_queries[0]);
	query_store.insert(begin(query_store), begin(gl_queries), end(gl_queries));
	enabled = true;
}
void gl_timer::destroy() {
	if(!enabled) return;
	glDeleteQueries((GLsizei)gl_queries.size(), &gl_queries[0]);
}
void gl_timer::state_check() {
	if(!enabled) return;
	
	for(auto& frame : frames) {
		if(!frame.done) continue;
		if(frame.done && frame.available) continue;
		
		GLint available = 1;
		for(const auto& qry : frame.queries) {
			glGetQueryObjectiv(qry.query_ref, GL_QUERY_RESULT_AVAILABLE, &available);
			if(available == 0) break;
		}
		if(available == 1) {
			for(auto& qry : frame.queries) {
				glGetQueryObjectui64v(qry.query_ref, GL_QUERY_RESULT, &qry.time);
				query_store.emplace_back(qry.query_ref);
			}
			frame.available = true;
		}
	}
}

void gl_timer::start_frame() {
	if(!enabled) return;
	
	// TODO: correct cleanup (wait for query?)
	if(frames[cur_frame].done && !frames[cur_frame].available) {
		log_error("overwriting frame query that is done, but not available yet!");
		for(const auto& qry : frames[cur_frame].queries) {
			query_store.emplace_back(qry.query_ref);
		}
	}
	frames[cur_frame].queries.clear();
	frames[cur_frame].done = false;
	frames[cur_frame].available = false;
	
	mark("FRAME_START");
}

void gl_timer::mark(const string& identifier) {
	if(!enabled) return;
	//glFlush();
	//glFinish();
	
	// random color through dumb string hash
	unsigned int dumb_hash = accumulate(cbegin(identifier), cend(identifier), 0x41324554,
										[](const unsigned int& cur_hash, const unsigned char& ch) {
											return (cur_hash * (unsigned int)ch) * 16807;
	});
	dumb_hash >>= 9;
	float3 color;
	color[dumb_hash % 3] = 1.0f;
	color[(dumb_hash + 1) % 3] = float((dumb_hash & 0x7F) + 0x80) / 255.0f;
	color[(dumb_hash + 2) % 3] = float(((dumb_hash >> 7) & 0x7F) + 0x80) / 255.0f;
	
	// get a query object from the query store and queue it
	const unsigned int gl_query { query_store.back() };
	query_store.pop_back();
	glQueryCounter(gl_query, GL_TIMESTAMP);
	frames[cur_frame].queries.emplace_back(frame_info::query_object { identifier, gl_query, color, 0 });
	//glFlush();
	//glFinish();
}

void gl_timer::stop_frame() {
	if(!enabled) return;
	mark("FRAME_STOP");
	frames[cur_frame].done = true;
	cur_frame = (cur_frame + 1) % stored_frames;
}

const gl_timer::frame_info* gl_timer::get_last_available_frame() {
	if(!enabled) return nullptr;
	for(size_t i = 0; i < stored_frames; i++) {
		const auto& frame(frames[(stored_frames + cur_frame - i - 1) % stored_frames]);
		if(frame.available && frame.done) return &frame;
	}
	return nullptr;
}
