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

#ifndef __EXTENSIONS_H__
#define __EXTENSIONS_H__

#include "global.h"

#include "core/core.h"
#if !(defined(A2E_EXTENSIONS_DONT_INCLUDE_GL_FUNCS) || defined(A2E_IOS))
#include "rendering/gl_funcs.h"
#endif

#if defined(A2E_IOS)
#include "rendering/gles_compat.h"
#endif

#ifndef __APPLE__
#define A2E_DEFINE_EXTENSIONS
#endif

/*! @class ext
 *  @brief opengl extensions
 */

class A2E_API ext {
public:
	ext(unsigned int imode, string* disabled_extensions, string* force_device, string* force_vendor);
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


	// - taken from engine.h -
    // the initialization mode is used to determine if we should load
    // or compute graphical stuff like textures or shaders
	enum INIT_MODE {
	    GRAPHICAL,
	    CONSOLE
	};

	enum GRAPHIC_CARD_VENDOR {
		GCV_UNKNOWN,
		GCV_NVIDIA,
		GCV_ATI,
		GCV_POWERVR
	};

	// these are only the most important and widely used models
	enum GRAPHIC_CARD {
		GC_UNKNOWN,
		GC_GENERIC_SM4,
		GC_GENERIC_SM5,
		GC_GEFORCE_8,
		GC_GEFORCE_9,
		GC_GEFORCE_GT200,
		GC_GEFORCE_GF100,
		GC_GEFORCE_GK100,
		GC_RADEON_HD2,
		GC_RADEON_HD3,
		GC_RADEON_HD4,
		GC_RADEON_HD5,
		GC_RADEON_HD6,
		GC_RADEON_HD7,
		GC_SGX_535,
		GC_SGX_543,
	};
	static const GRAPHIC_CARD min_generic_card = GC_GENERIC_SM4;
	static const GRAPHIC_CARD max_generic_card = GC_GENERIC_SM5;
	static const GRAPHIC_CARD min_nvidia_card = GC_GEFORCE_8;
	static const GRAPHIC_CARD max_nvidia_card = GC_GEFORCE_GK100;
	static const GRAPHIC_CARD min_ati_card = GC_RADEON_HD2;
	static const GRAPHIC_CARD max_ati_card = GC_RADEON_HD7;
	static const GRAPHIC_CARD min_powervr_card = GC_SGX_535;
	static const GRAPHIC_CARD max_powervr_card = GC_SGX_543;

	GRAPHIC_CARD_VENDOR get_vendor();
	GRAPHIC_CARD get_graphic_card();

	static const char* GRAPHIC_CARD_STR[];
	static const char* GRAPHIC_CARD_VENDOR_DEFINE_STR[];
	static const char* GRAPHIC_CARD_DEFINE_STR[];
	
	enum OPENGL_VERSION {
		OPENGL_UNKNOWN,
		OPENGL_ES_2_0,
		OPENGL_3_0,
		OPENGL_3_1,
		OPENGL_3_2,
		OPENGL_3_3,
		OPENGL_4_0,
		OPENGL_4_1,
		OPENGL_4_2,
	};
	
	enum GLSL_VERSION {
		GLSL_NO_VERSION,	// used when none is applicable
		GLSL_ES_100,		// opengl es 2.0
		GLSL_150,			// opengl 3.2
		GLSL_330,			// opengl 3.3
		GLSL_400,			// opengl 4.0
		GLSL_410,			// opengl 4.1
		GLSL_420,			// opengl 4.2
	};
	
	const char* glsl_version_str_from_glsl_version(const GLSL_VERSION& version) const;
	const char* cstr_from_glsl_version(const GLSL_VERSION& version) const;
	GLSL_VERSION to_glsl_version(const size_t& major_version, const size_t& minor_version) const;
	GLSL_VERSION to_glsl_version(const size_t& version) const;
	
	const char* cstr_from_gl_version(const OPENGL_VERSION& version) const;

protected:
	unsigned int mode;
	
	OPENGL_VERSION opengl_version;
	GLSL_VERSION glsl_version;
	
	void init_gl_funcs();
	void check_gl_funcs();

	bool shader_support;
	bool shader_model_5_0_support;
	bool anisotropic_filtering_support;
	bool fbo_multisample_coverage_support;

	unsigned int max_anisotropic_filtering;
	unsigned int max_texture_size;
	unsigned int max_texture_image_units;
	unsigned int max_samples;
	unsigned int max_multisample_coverage_modes;
	unsigned int max_draw_buffers;

	struct NV_MULTISAMPLE_COVERAGE_MODE {
		int coverage_samples;
		int color_samples;
	};
	NV_MULTISAMPLE_COVERAGE_MODE* multisample_coverage_modes;

	string* disabled_extensions;
	string* force_device;
	string* force_vendor;

	GRAPHIC_CARD_VENDOR vendor;
	GRAPHIC_CARD graphic_card;
	
	set<string> supported_extensions;

};

#endif
