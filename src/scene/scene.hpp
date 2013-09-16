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

#ifndef __A2E_SCENE_H__
#define __A2E_SCENE_H__

#define A2E_CONCURRENT_FRAMES 1
//#define A2E_INFERRED_RENDERING_CL 1

#include "global.hpp"

#include "core/core.hpp"
#include "engine.hpp"
#include "scene/model/a2estatic.hpp"
#include "scene/model/a2emodel.hpp"
#include "scene/light.hpp"
#include "rendering/shader.hpp"
#include "core/matrix4.hpp"
#include "core/bbox.hpp"
#include "rendering/rtt.hpp"
#include "cl/opencl.hpp"

/*! @class scene
 *  @brief a2e scene manager
 */

class particle_manager;

class A2E_API scene {
protected:
	struct frame_buffers {
		rtt::fbo* scene_buffer = nullptr; // final output buffer
		rtt::fbo* fxaa_buffer = nullptr; // fxaa
		rtt::fbo* g_buffer[2] = { nullptr, nullptr }; // opaque + alpha
		rtt::fbo* l_buffer[2] = { nullptr, nullptr }; // opaque + alpha
		
#if defined(A2E_INFERRED_RENDERING_CL)
		struct cl_frame_buffers {
			// buffers
			opencl::buffer_object* geometry_buffer[2] = { nullptr, nullptr };
			opencl::buffer_object* depth_buffer[2] = { nullptr, nullptr };
			opencl::buffer_object* light_buffer[4] = { nullptr, nullptr, nullptr, nullptr };
			
			// light data storage
			opencl::buffer_object* lights_buffer = nullptr;
			struct __attribute__((aligned(32), packed)) ir_light {
				float4 position;
				float4 color;
			};
			static constexpr size_t max_ir_lights = 128;
			array<ir_light, max_ir_lights> ir_lights;
			
			// used to copy the gl depth buffer to a cl readable float buffer
			GLuint depth_copy_tex = 0;
			GLuint depth_copy_fbo = 0;
			
			//
			opencl::buffer_object* _dbg_buffer = nullptr;
		} cl;
#endif
	};
	
public:
	scene(engine* e);
	~scene();

	void draw();
	
	void set_enabled(const bool& status);
	bool is_enabled() const;
	
	template<typename T> T* create_a2emodel();
	void add_model(a2emodel* model);
	void delete_model(a2emodel* model);
	void add_light(light* light);
	void delete_light(light* light);
	void add_particle_manager(particle_manager* pm);
	void delete_particle_manager(particle_manager* pm);
	
	typedef functor<void, const DRAW_MODE> draw_callback;
	void add_draw_callback(const string& name, draw_callback& cb);
	void delete_draw_callback(draw_callback& cb);
	void delete_draw_callback(const string& name);

	void set_skybox_texture(a2e_texture tex);
	const a2e_texture& get_skybox_texture() const;
	void set_render_skybox(const bool state);
	bool get_render_skybox() const;
	
	void set_eye_distance(const float& distance);
	const float& get_eye_distance() const;
	
	//
	void add_alpha_object(const extbbox* bbox, const size_t& sub_object_id, a2emodel::draw_callback* cb);
	void add_alpha_objects(const size_t count, const extbbox** bboxes, const size_t* sub_object_ids, a2emodel::draw_callback* cbs);
	void delete_alpha_object(const extbbox* bbox);
	void delete_alpha_objects(const size_t count, const extbbox** bboxes);
	
	// post-processing
	typedef functor<void, rtt::fbo*> post_processing_handler;
	void add_post_processing(post_processing_handler* pph);
	void delete_post_processing(const post_processing_handler* pph);
	
	// environment probing/mapping
	struct env_probe {
		float3 position;
		float2 rotation;
		const size2 buffer_size;
		const bool capture_alpha;
		frame_buffers buffers;
		size_t frame_freq = 1;
		size_t frame_counter = frame_freq;
		
		enum class PROBE_FREQUENCY : unsigned int {
			ONCE,
			EVERY_FRAME,
			NTH_FRAME
		};
		PROBE_FREQUENCY freq = PROBE_FREQUENCY::ONCE;
		
		void set_probe_frequency(const PROBE_FREQUENCY freq_) {
			freq = freq_;
		}
		//! only use this in combination with NTH_FRAME
		void set_frame_frequency(const size_t& frame_freq_) {
			frame_freq = frame_freq_;
		}
		
		env_probe(const float3& pos, const float2& rot, const size2 buffer_size, const bool capture_alpha);
		~env_probe();
	};
	env_probe* add_environment_probe(const float3& pos, const float2& rot, const size2 buffer_size, const bool capture_alpha);
	void add_environment_probe(env_probe* probe);
	void delete_environment_probe(env_probe* probe);
	
	// for debugging and other evil purposes:
	const frame_buffers& get_frame_buffers(const size_t num = 0) const { return frames[num]; }
	const rtt::fbo* get_geometry_buffer(const size_t type = 0) const { return frames[0].g_buffer[type]; }
	const rtt::fbo* get_light_buffer(const size_t type = 0) const { return frames[0].l_buffer[type]; }
	const rtt::fbo* get_fxaa_buffer() const { return frames[0].fxaa_buffer; }
	const rtt::fbo* get_scene_buffer() const { return frames[0].scene_buffer; }
	
	const vector<a2emodel*>& get_models() const;
	const vector<light*>& get_lights() const;
	const vector<particle_manager*>& get_particle_managers() const;
	const set<env_probe*>& get_env_probes() const;

protected:
	engine* e;
	shader* s;
	ext* exts;
	rtt* r;
	opencl_base* cl;
	
	void setup_scene();
	void geometry_pass(frame_buffers& buffers, const DRAW_MODE draw_mode_or_mask = DRAW_MODE::NONE);
	void light_and_material_pass(frame_buffers& buffers, const DRAW_MODE draw_mode_or_mask = DRAW_MODE::NONE);
	void postprocess();
	void sort_alpha_objects();
	void delete_buffers(frame_buffers& buffers);
	void recreate_buffers(frame_buffers& buffers, const size2 buffer_size, const bool create_alpha_buffer = true);

	//
	vector<a2emodel*> models;
	vector<light*> lights;
	vector<particle_manager*> particle_managers;
	set<env_probe*> env_probes;
	
	// <bbox*, <sub-object id, draw functor*>>
	map<const extbbox*, pair<size_t, a2emodel::draw_callback*>> alpha_objects;
	// <bbox*, mask id>, mask id: 0 (invalid), {1, 2, 3}
	vector<pair<const extbbox*, size_t>> sorted_alpha_objects;
	
	bool enabled = true;

	a2e_texture skybox_tex = nullptr;
	bool render_skybox = false;

	a2estatic* light_sphere = nullptr;

	// render and scene buffer
	frame_buffers frames[A2E_CONCURRENT_FRAMES];
	//size_t cur_frame = 0;
	
	vector<post_processing_handler*> pp_handlers;
	map<string, draw_callback*> draw_callbacks;

	// stereo rendering (currently unsupported)
	float eye_distance = -0.3f; // 1.5f?
	bool stereo = false;
	
	// event handlers
	event::handler window_handler;
	bool window_event_handler(EVENT_TYPE type, shared_ptr<event_object> obj);

};

/*! creates an a2emodel object and returns it
 */
template<typename T> T* scene::create_a2emodel() {
	return new T(e, s, this);
}

#endif
