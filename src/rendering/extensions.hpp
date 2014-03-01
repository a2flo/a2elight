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

#ifndef __A2E_EXTENSIONS_HPP__
#define __A2E_EXTENSIONS_HPP__

#include "global.hpp"

#include "core/core.hpp"
#if !(defined(A2E_EXTENSIONS_DONT_INCLUDE_GL_FUNCS) || defined(FLOOR_IOS))
#include "rendering/gl_funcs.hpp"
#endif

#if defined(FLOOR_IOS)
#include "core/gl_support.hpp"
#endif

#if !defined(__APPLE__)
#define A2E_DEFINE_EXTENSIONS
#endif

/*! @class ext
 *  @brief opengl extensions
 */

class A2E_API ext {
public:
	ext(string* disabled_extensions, string* force_device, string* force_vendor);
	~ext();

	bool is_ext_supported(const string& ext_name);
	bool is_gl_version(unsigned int major, unsigned int minor);
	bool is_shader_support();
	bool is_shader_model_5_0_support();
	bool is_anisotropic_filtering_support();
	bool is_fbo_multisample_coverage_support();
	bool is_fbo_multisample_coverage_mode_support(unsigned int coverage_samples, unsigned int color_samples);

	unsigned int get_max_anisotropic_filtering();
	unsigned int get_max_texture_size();
	unsigned int get_max_texture_image_units();
	unsigned int get_max_samples();
	unsigned int get_max_draw_buffers();

	enum class GRAPHICS_CARD_VENDOR : unsigned int {
		UNKNOWN,
		NVIDIA,
		ATI,
		POWERVR,
		INTEL,
		APPLE,
	};

	// these are only the most important and widely used models
	enum class GRAPHICS_CARD : unsigned int {
		UNKNOWN,
		GENERIC_SM4,
		GENERIC_SM5,
		GEFORCE_8,
		GEFORCE_9,
		GEFORCE_GT200,
		GEFORCE_GF100,
		GEFORCE_GK100,
		RADEON_HD2,
		RADEON_HD3,
		RADEON_HD4,
		RADEON_HD5,
		RADEON_HD6,
		RADEON_HD7,
		RADEON_HD8,
		SGX_535,
		SGX_543,
		SGX_554,
		IVY_BRIDGE,
		HASWELL,
		APPLE_A7,
	};
	static const GRAPHICS_CARD min_generic_card = GRAPHICS_CARD::GENERIC_SM4;
	static const GRAPHICS_CARD max_generic_card = GRAPHICS_CARD::GENERIC_SM5;
	static const GRAPHICS_CARD min_nvidia_card = GRAPHICS_CARD::GEFORCE_8;
	static const GRAPHICS_CARD max_nvidia_card = GRAPHICS_CARD::GEFORCE_GK100;
	static const GRAPHICS_CARD min_ati_card = GRAPHICS_CARD::RADEON_HD2;
	static const GRAPHICS_CARD max_ati_card = GRAPHICS_CARD::RADEON_HD8;
	static const GRAPHICS_CARD min_powervr_card = GRAPHICS_CARD::SGX_535;
	static const GRAPHICS_CARD max_powervr_card = GRAPHICS_CARD::SGX_554;
	static const GRAPHICS_CARD min_intel_card = GRAPHICS_CARD::IVY_BRIDGE;
	static const GRAPHICS_CARD max_intel_card = GRAPHICS_CARD::HASWELL;
	static const GRAPHICS_CARD min_apple_card = GRAPHICS_CARD::APPLE_A7;
	static const GRAPHICS_CARD max_apple_card = GRAPHICS_CARD::APPLE_A7;

	GRAPHICS_CARD_VENDOR get_vendor();
	GRAPHICS_CARD get_graphics_card();

	static const char* GRAPHICS_CARD_STR[];
	static const char* GRAPHICS_CARD_VENDOR_DEFINE_STR[];
	static const char* GRAPHICS_CARD_DEFINE_STR[];
	
	enum class OPENGL_VERSION {
		OPENGL_UNKNOWN,
		OPENGL_ES_2_0,
		OPENGL_ES_3_0,
		OPENGL_3_0,
		OPENGL_3_1,
		OPENGL_3_2,
		OPENGL_3_3,
		OPENGL_4_0,
		OPENGL_4_1,
		OPENGL_4_2,
		OPENGL_4_3,
		OPENGL_4_4,
	};
	
	enum class GLSL_VERSION {
		GLSL_NO_VERSION,	// used when none is applicable
		GLSL_ES_100,		// opengl es 2.0
		GLSL_ES_300,		// opengl es 3.0
		GLSL_150,			// opengl 3.2
		GLSL_330,			// opengl 3.3
		GLSL_400,			// opengl 4.0
		GLSL_410,			// opengl 4.1
		GLSL_420,			// opengl 4.2
		GLSL_430,			// opengl 4.3
		GLSL_440,			// opengl 4.4
	};
	
	const char* glsl_version_str_from_glsl_version(const GLSL_VERSION& version) const;
	const char* cstr_from_glsl_version(const GLSL_VERSION& version) const;
	GLSL_VERSION to_glsl_version(const size_t& major_version, const size_t& minor_version) const;
	GLSL_VERSION to_glsl_version(const size_t& version) const;
	
	const char* cstr_from_gl_version(const OPENGL_VERSION& version) const;

protected:
	OPENGL_VERSION opengl_version;
	GLSL_VERSION glsl_version;
	
	void init_gl_funcs();
	void check_gl_funcs();

	bool shader_support { false };
	bool shader_model_5_0_support { false };
	bool anisotropic_filtering_support { false };
	bool fbo_multisample_coverage_support { false };

	unsigned int max_anisotropic_filtering { 0 };
	unsigned int max_texture_size { 0 };
	unsigned int max_texture_image_units { 0 };
	unsigned int max_samples { 0 };
	unsigned int max_multisample_coverage_modes { 0 };
	unsigned int max_draw_buffers { 0 };

	struct NV_MULTISAMPLE_COVERAGE_MODE {
		int coverage_samples;
		int color_samples;
	};
	vector<NV_MULTISAMPLE_COVERAGE_MODE> multisample_coverage_modes;

	string* disabled_extensions;
	string* force_device;
	string* force_vendor;

	GRAPHICS_CARD_VENDOR vendor;
	GRAPHICS_CARD graphics_card;
	
	set<string> supported_extensions;

};

#endif
