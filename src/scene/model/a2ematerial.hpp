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

#ifndef __A2E_A2EMATERIAL_HPP__
#define __A2E_A2EMATERIAL_HPP__

#include "global.hpp"

#include "core/core.hpp"
#include "core/file_io.hpp"
#include "core/xml.hpp"
#include "engine.hpp"
#include "rendering/extensions.hpp"
#include "rendering/shader.hpp"

#define A2E_MATERIAL_VERSION 2

//! a2e material routines
class a2ematerial {
public:
	a2ematerial();
	~a2ematerial();
	
	//// generic lighting/material definitions
	enum class LIGHTING_MODEL : unsigned int {
		NONE,
		PHONG,
		ASHIKHMIN_SHIRLEY
	};
	enum class MATERIAL_TYPE : unsigned int {
		NONE,
		DIFFUSE,
		PARALLAX
	};
	enum class TEXTURE_TYPE : unsigned int {
		DIFFUSE			= (1 << 0),
		SPECULAR		= (1 << 1),
		REFLECTANCE		= (1 << 2),
		HEIGHT			= (1 << 3),
		NORMAL			= (1 << 4),
		ANISOTROPIC		= (1 << 5),
	};
	enum_class_bitwise_or(TEXTURE_TYPE)
	enum_class_bitwise_and(TEXTURE_TYPE)

	struct lighting_model;
	struct material_object;
	struct material {
		ssize_t id = -1;
		MATERIAL_TYPE mat_type = MATERIAL_TYPE::NONE;
		material_object* mat = nullptr;
		LIGHTING_MODEL lm_type = LIGHTING_MODEL::PHONG;
		lighting_model* model = nullptr;
		
		material() {}
	};
	
	//// lighting models
	struct lighting_model {
		lighting_model() {}
	};
	
	struct phong_model : public lighting_model {
		phong_model() {}
	};
	
	struct ashikhmin_shirley_model : public lighting_model {
		a2e_texture anisotropic_texture = nullptr;
		float2 anisotropic_roughness = float2(1.0f, 1.0f);
		ashikhmin_shirley_model() : lighting_model() {}
	};
	
	//// material types
	struct material_object {
		material_object() {}
	};
	struct diffuse_material : public material_object {
		a2e_texture diffuse_texture = nullptr;
		a2e_texture specular_texture = nullptr;
		a2e_texture reflectance_texture = nullptr;
		diffuse_material() : material_object() {}
	};
	struct parallax_material : public diffuse_material {
		a2e_texture height_texture = nullptr;
		a2e_texture normal_texture = nullptr;
		bool parallax_occlusion = false;
		parallax_material() : diffuse_material() {}
	};

	//// functions
	void load_material(const string& filename);
	const string& get_filename() const;
	
	bool is_blending(const size_t& object_id) const;
	bool is_parallax_occlusion(const size_t& object_id) const;
	MATERIAL_TYPE get_material_type(const size_t& object_id) const;
	LIGHTING_MODEL get_lighting_model_type(const size_t& object_id) const;
	const lighting_model* get_lighting_model(const size_t& object_id) const;
	const material& get_material(const size_t& material_id) const;
	material& get_material(const size_t& material_id);
	size_t get_material_count() const;

	void enable_textures(const size_t& object_id, gl_shader& shd, const TEXTURE_TYPE texture_mask = (TEXTURE_TYPE)~(unsigned int)0) const;
	void disable_textures(const size_t& object_id) const;
	
	void copy_object_mapping(const size_t& from_object, const size_t& to_object);
	void copy_object_mapping(const size_t& from_object, const vector<size_t>& to_objects);

protected:
	texman* t;
	ext* exts;
	xml* x;
	
	string filename = "";
	
	stringstream buffer;
	
	float4 get_color(const string& color_str);
	
	struct object_mapping {
		material* mat;
		bool blending;
		object_mapping() : mat(nullptr), blending(false) {}
		object_mapping(material* mat_, bool blending_) : mat(mat_), blending(blending_) {}
	};
	const object_mapping* get_object_mapping(const size_t& object_id) const;
	// <obj id, object>
	map<size_t, object_mapping*> mapping;
	vector<material> materials;
	
	a2e_texture dummy_texture;
	a2e_texture default_specular;

};

#endif
