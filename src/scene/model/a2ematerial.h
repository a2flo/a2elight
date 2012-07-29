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

#ifndef __A2E_A2EMATERIAL_H__
#define __A2E_A2EMATERIAL_H__

#include "global.h"

#include "core/core.h"
#include "core/file_io.h"
#include "core/xml.h"
#include "engine.h"
#include "rendering/extensions.h"
#include "rendering/shader.h"

#define A2E_MATERIAL_VERSION 2

/*! @class a2ematerial
 *  @brief a2e material routines
 */

class A2E_API a2ematerial {
public:
	a2ematerial(engine* eng);
	~a2ematerial();
	
	//// generic lighting/material definitions
	enum LIGHTING_MODEL {
		LM_NONE,
		LM_PHONG,
		LM_WARD,
		LM_ASHIKHMIN_SHIRLEY
	};
	enum MATERIAL_TYPE {
		NONE,
		DIFFUSE,
		PARALLAX
	};
	enum TEXTURE_TYPE {
		TT_DIFFUSE		= 1 << 0,
		TT_SPECULAR		= 1 << 1,
		TT_REFLECTANCE	= 1 << 2,
		TT_HEIGHT		= 1 << 3,
		TT_NORMAL		= 1 << 4,
		TT_ISOTROPIC	= 1 << 5,
		TT_ANISOTROPIC	= 1 << 6
	};

	struct lighting_model;
	struct material_object;
	struct material {
		ssize_t id = -1;
		MATERIAL_TYPE mat_type = NONE;
		material_object* mat = nullptr;
		LIGHTING_MODEL lm_type = LM_PHONG;
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
	
	enum WARD_TYPE {
		WT_ISOTROPIC,
		WT_ANISOTROPIC
	};
	struct ward_model : public lighting_model {
		WARD_TYPE type = WT_ISOTROPIC;
		a2e_texture isotropic_texture = nullptr;
		a2e_texture anisotropic_texture = nullptr;
		float isotropic_roughness = 1.0f;
		float2 anisotropic_roughness = float2(1.0f, 1.0f);
		ward_model() : lighting_model() {}
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

	void enable_textures(const size_t& object_id, gl3shader& shd, const size_t texture_mask = ~(size_t)0) const;
	void disable_textures(const size_t& object_id) const;
	
	void copy_object_mapping(const size_t& from_object, const size_t& to_object);
	void copy_object_mapping(const size_t& from_object, const vector<size_t>& to_objects);

protected:
	engine* e;
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
