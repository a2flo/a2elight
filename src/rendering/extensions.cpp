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

#include "extensions.hpp"
#include "engine.hpp"

// i won't add quadro or firegl cards here, since their naming scheme is inscrutable.
// users will have to use the "force profile" option in the config.json (set to the
// appropriate geforce or radeon card) or simply use the "Generic SM x.0" profile.
// i won't add older cards here, b/c they are incapable of doing shaders and don't
// need any specific render paths which aren't already used via standard extension
// checking. i also won't add cards of other vendors, since i don't own any or could
// otherwise check what they are capable of. they are most certainly unimportant
// anyways and can simply use the generic profile.
const char* ext::GRAPHICS_CARD_STR[] = {
	"Unknown",
	"Generic SM 4.0",
	"Generic SM 5.0",
	"GeForce 8",
	"GeForce 9",
	"GeForce GT200",
	"GeForce GF100",
	"GeForce GK100",
	"GeForce GM100",
	"Radeon HD2",
	"Radeon HD3",
	"Radeon HD4",
	"Radeon HD5",
	"Radeon HD6",
	"Radeon HD7",
	"Radeon HD8",
	"PowerVR SGX535",
	"PowerVR SGX543",
	"PowerVR SGX554",
	"Ivy Bridge",
	"Haswell",
	"Apple A7",
};

const char* ext::GRAPHICS_CARD_VENDOR_DEFINE_STR[] = {
	"UNKNOWN",
	"NVIDIA",
	"ATI",
	"POWERVR",
	"INTEL",
	"APPLE",
};

const char* ext::GRAPHICS_CARD_DEFINE_STR[] = {
	"UNKNOWN",
	"GENERIC_SM_4_0",
	"GENERIC_SM_5_0",
	"GEFORCE_8",
	"GEFORCE_9",
	"GEFORCE_GT200",
	"GEFORCE_GF100",
	"GEFORCE_GK100",
	"GEFORCE_GM100",
	"RADEON_HD2",
	"RADEON_HD3",
	"RADEON_HD4",
	"RADEON_HD5",
	"RADEON_HD6",
	"RADEON_HD7",
	"RADEON_HD8",
	"SGX_535",
	"SGX_543",
	"SGX_554",
	"IVY_BRIDGE",
	"HASWELL",
	"APPLE_A7",
};

/*! create and initialize the extension class
 */
ext::ext(string* disabled_extensions_, string* force_device_, string* force_vendor_) {
	ext::disabled_extensions = disabled_extensions_;
	ext::force_device = force_device_;
	ext::force_vendor = force_vendor_;
	
	// get supported extensions
#if !defined(FLOOR_IOS)
	unsigned int ext_count = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, (int*)&ext_count);
	for(unsigned int i = 0; i < ext_count; i++) {
		supported_extensions.insert((const char*)glGetStringi(GL_EXTENSIONS, i));
	}
#else
	const string exts = (const char*)glGetString(GL_EXTENSIONS);
	for(size_t pos = 0; (pos = exts.find("GL_", pos)) != string::npos; pos++) {
		supported_extensions.emplace(exts.substr(pos, exts.find(" ", pos) - pos));
	}
#endif
	
	// get opengl and glsl version
	const string str_gl_version = (const char*)glGetString(GL_VERSION);
	const string str_glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	
	opengl_version = OPENGL_VERSION::OPENGL_UNKNOWN;
#if !defined(FLOOR_IOS)
	switch(str_gl_version[0]) {
		case '0':
		case '1':
		case '2':
			opengl_version = OPENGL_VERSION::OPENGL_UNKNOWN;
			break;
		case '3':
			opengl_version = OPENGL_VERSION::OPENGL_3_0;
			switch(str_gl_version[2]) {
				case '0': break;
				case '1':
					opengl_version = OPENGL_VERSION::OPENGL_3_1;
					break;
				case '2':
					opengl_version = OPENGL_VERSION::OPENGL_3_2;
					break;
				case '3':
				default:
					opengl_version = OPENGL_VERSION::OPENGL_3_3;
					break;
			}
			break;
		case '4':
			opengl_version = OPENGL_VERSION::OPENGL_4_0;
			switch(str_gl_version[2]) {
				case '0': break;
				case '1':
					opengl_version = OPENGL_VERSION::OPENGL_4_1;
					break;
				case '2':
					opengl_version = OPENGL_VERSION::OPENGL_4_2;
					break;
				case '3':
					opengl_version = OPENGL_VERSION::OPENGL_4_3;
					break;
				case '4':
					opengl_version = OPENGL_VERSION::OPENGL_4_4;
					break;
				case '5':
				default:
					opengl_version = OPENGL_VERSION::OPENGL_4_5;
					break;
			}
			break;
		default:
			// default to highest version
			opengl_version = OPENGL_VERSION::OPENGL_4_5;
			break;
	}
#else
	const string str_gles_version = str_gl_version.substr(str_gl_version.find("OpenGL ES ")+10,
														  str_gl_version.length()-10);
	switch(str_gles_version[0]) {
		case '0':
		case '1':
			opengl_version = OPENGL_VERSION::OPENGL_UNKNOWN;
			break;
		case '2':
			opengl_version = OPENGL_VERSION::OPENGL_ES_2_0;
			break;
		case '3':
			switch(str_glsl_version[2]) {
				case '0':
					opengl_version = OPENGL_VERSION::OPENGL_ES_3_0;
					break;
				case '1':
				default:
					opengl_version = OPENGL_VERSION::OPENGL_ES_3_1;
					break;
			}
			break;
		default:
			// default to highest version
			opengl_version = OPENGL_VERSION::OPENGL_ES_3_1;
			break;
	}
#endif
	
	glsl_version = GLSL_VERSION::GLSL_NO_VERSION;
#if !defined(FLOOR_IOS)
	switch(str_glsl_version[0]) {
		case '1':
			switch(str_glsl_version[2]) {
				case '5': glsl_version = GLSL_VERSION::GLSL_150; break;
				default: break;
			}
			break;
		case '3':
			switch(str_glsl_version[2]) {
				case '3':
				default:
					glsl_version = GLSL_VERSION::GLSL_330;
					break;
			}
			break;
		case '4':
			switch(str_glsl_version[2]) {
				case '0': glsl_version = GLSL_VERSION::GLSL_400; break;
				case '1': glsl_version = GLSL_VERSION::GLSL_410; break;
				case '2': glsl_version = GLSL_VERSION::GLSL_420; break;
				case '3': glsl_version = GLSL_VERSION::GLSL_430; break;
				case '4': glsl_version = GLSL_VERSION::GLSL_440; break;
				case '5':
				default:
					glsl_version = GLSL_VERSION::GLSL_450;
					break;
			}
			break;
		default:
			// default to highest version
			glsl_version = GLSL_VERSION::GLSL_450;
			break;
	}
#else
#if defined(PLATFORM_X64)
	glsl_version = GLSL_VERSION::GLSL_ES_300;
#else
	glsl_version = GLSL_VERSION::GLSL_ES_100;
#endif
#endif
	shader_support = (glsl_version >= GLSL_VERSION::GLSL_ES_100);
	
	const auto imode = engine::get_init_mode();
	if(imode == engine::INIT_MODE::GRAPHICAL) {
		string vendor_str = "";
		if(*force_vendor != "") {
			vendor_str = *force_vendor;
		}
		else {
			const char* cvendor_string = (const char*)glGetString(GL_VENDOR);
			if(cvendor_string != nullptr) {
				vendor_str = cvendor_string;
			}
		}
		core::str_to_lower_inplace(vendor_str);
		if(vendor_str.find("nvidia") != string::npos) vendor = ext::GRAPHICS_CARD_VENDOR::NVIDIA;
		// imagin(ati)on needs to be checked first ...
		else if(vendor_str.find("imagination") != string::npos) vendor = ext::GRAPHICS_CARD_VENDOR::POWERVR;
		else if(vendor_str.find("ati") != string::npos) vendor = ext::GRAPHICS_CARD_VENDOR::ATI;
		else if(vendor_str.find("intel") != string::npos) vendor = ext::GRAPHICS_CARD_VENDOR::INTEL;
		else if(vendor_str.find("apple") != string::npos) vendor = ext::GRAPHICS_CARD_VENDOR::APPLE;
		else vendor = ext::GRAPHICS_CARD_VENDOR::UNKNOWN;
	}
	else {
		vendor = ext::GRAPHICS_CARD_VENDOR::UNKNOWN;
		graphics_card = ext::GRAPHICS_CARD::UNKNOWN;
	}
	
	if(imode == engine::INIT_MODE::GRAPHICAL) {
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&max_texture_size);
	}

	// check extension support and set flags accordingly
	const struct {
		bool* flag;
		const vector<string> exts;
	} ext_checks[] {
#if !defined(FLOOR_IOS)
		{ &fbo_multisample_coverage_support, { "GL_NV_framebuffer_multisample_coverage" } },
		{ &shader_model_5_0_support, { "GL_ARB_gpu_shader5" } },
#endif
		{ &anisotropic_filtering_support, { "GL_EXT_texture_filter_anisotropic" } },
	};
	
	for(const auto& check : ext_checks) {
		if(*check.flag == true) continue; // already supported by another extension
		*check.flag = true;
		for(const auto& ext_str : check.exts) {
			if(!is_ext_supported(ext_str)) {
				*check.flag = false;
				if(imode == engine::INIT_MODE::GRAPHICAL) {
					log_msg("your graphic device doesn't support '%s'!", ext_str);
				}
				// don't break here, but rather print all extensions that aren't supported
			}
		}
	}

	//////
	if(!shader_model_5_0_support && glsl_version >= GLSL_VERSION::GLSL_400) {
		shader_model_5_0_support = true;
	}

	// get max multi-sampling samples
#if !defined(FLOOR_IOS) && 0 // TODO: fix or remove this
	if(fbo_multisample_coverage_support) {
		glGetIntegerv(GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV, (GLint*)&max_multisample_coverage_modes);
		multisample_coverage_modes.clear();
		multisample_coverage_modes.resize(max_multisample_coverage_modes);
		glGetIntegerv(GL_MULTISAMPLE_COVERAGE_MODES_NV, (GLint*)&multisample_coverage_modes[0]);
	}
#endif
	if(imode == engine::INIT_MODE::GRAPHICAL) glGetIntegerv(GL_MAX_SAMPLES, (GLint*)&max_samples);
	
	// get max vertex textures
	GLint vtf = 0;
	if(imode == engine::INIT_MODE::GRAPHICAL) glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &vtf);
	if(vtf > 0) {
		log_debug("supported vertex textures: %i", vtf);
	}
	else {
		if(imode == engine::INIT_MODE::GRAPHICAL) log_error("your graphic device doesn't support 'Vertex Texture Fetching'!");
	}
	
	// get max anisotropic filtering
	if(anisotropic_filtering_support) {
		max_anisotropic_filtering = 1;
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, (GLint*)(&max_anisotropic_filtering));
	}
	
	// get max textures
	if(imode == engine::INIT_MODE::GRAPHICAL) glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*)&(ext::max_texture_image_units));
	
	// get max draw buffers
#if !defined(FLOOR_IOS) || defined(PLATFORM_X64)
	if(imode == engine::INIT_MODE::GRAPHICAL) glGetIntegerv(GL_MAX_DRAW_BUFFERS, (GLint*)&(ext::max_draw_buffers));
#else
	// no support on opengl es 2.0 on iOS
	max_draw_buffers = 1;
#endif

	// graphic card handling
	if(imode == engine::INIT_MODE::GRAPHICAL) {
		graphics_card = (shader_support ? (shader_model_5_0_support ?
										   GRAPHICS_CARD::GENERIC_SM5 : GRAPHICS_CARD::GENERIC_SM4) : GRAPHICS_CARD::UNKNOWN);

		// get renderer string and make it lower case
		const char* gl_renderer_str = (const char*)glGetString(GL_RENDERER);
		if(gl_renderer_str == nullptr) {
			log_error("gl renderer str is nullptr - invalid opengl context?");
		}
		string renderer_str = (*force_device == "") ? (gl_renderer_str == nullptr ? "" : gl_renderer_str) : force_device->c_str();
		core::str_to_lower_inplace(renderer_str);
		if(vendor == ext::GRAPHICS_CARD_VENDOR::NVIDIA) {
			if(renderer_str.find("geforce 8") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::GEFORCE_8;
			}
			// yes, these are all actually geforce 9 cards ... and thank you nvidia for this totally fucked up naming scheme ...
			else if(renderer_str.find("geforce 9") != string::npos ||		// 9xxx(M)
					renderer_str.find("geforce 1") != string::npos ||		// 1xx(M)
					renderer_str.find("geforce g 1") != string::npos ||		// G 1xx(M)
					renderer_str.find("geforce g1") != string::npos ||		// G1xx(M)
					renderer_str.find("geforce gt 1") != string::npos ||	// GT 1xx(M)
					renderer_str.find("geforce gts 1") != string::npos ||	// GTS 1xx(M)
					(renderer_str.find("geforce gts 2") != string::npos &&	// GTS 2xx (!M)
					 renderer_str.find("m") == string::npos) ||
					(renderer_str.find("geforce gtx 2") != string::npos &&	// GTX 2xxM
					 renderer_str.find("m") != string::npos)) {
				graphics_card = ext::GRAPHICS_CARD::GEFORCE_9;
			}
			else if(renderer_str.find("geforce 2") != string::npos ||		// 2xx(M)
					renderer_str.find("geforce 3") != string::npos ||		// 3xx(M)
					renderer_str.find("geforce g 2") != string::npos ||		// G 2xx(M)
					renderer_str.find("geforce g2") != string::npos ||		// G2xx(M)
					renderer_str.find("geforce gt 2") != string::npos ||	// GT 2xx(M)
					renderer_str.find("geforce gts 2") != string::npos ||	// GTS 2xx(M)
					renderer_str.find("geforce gtx 2") != string::npos ||	// GTX 2xx
					renderer_str.find("geforce gt 3") != string::npos ||	// GT 3xx(M)
					renderer_str.find("geforce gts 3") != string::npos) {	// GTS 3xx(M)
				graphics_card = ext::GRAPHICS_CARD::GEFORCE_GT200;
			}
			else if(renderer_str.find("geforce gtx 4") != string::npos ||	// GTX 4xx
					renderer_str.find("geforce gts 4") != string::npos ||	// GTS 4xx
					renderer_str.find("geforce gt 4") != string::npos ||	// GT 4xx
					renderer_str.find("geforce gtx 5") != string::npos ||	// GTX 5xx
					renderer_str.find("geforce gts 5") != string::npos ||	// GTS 5xx
					renderer_str.find("geforce gt 5") != string::npos ||	// GT 5xx
					// this is getting out of hand ...
					(renderer_str.find("geforce gtx 6") != string::npos &&
					 renderer_str.find("m") != string::npos &&
					 renderer_str.find("660m") == string::npos &&
					 renderer_str.find("680m") == string::npos) ||			// GTX 6xxM (!660M/680M)
					(renderer_str.find("geforce gt 6") != string::npos &&
					 renderer_str.find("m") != string::npos &&
					 renderer_str.find("650m") == string::npos &&
					 renderer_str.find("640m") == string::npos) ||			// GT 6xxM (!640M/650M)
					(renderer_str.find("geforce gts 6") != string::npos &&
					 renderer_str.find("m") != string::npos)) {				// GTS 6xxM
				graphics_card = ext::GRAPHICS_CARD::GEFORCE_GF100;
			}
			else if(renderer_str.find("geforce gtx 6") != string::npos ||	// GTX 6xx
					renderer_str.find("geforce gts 6") != string::npos ||	// GTS 6xx
					renderer_str.find("geforce gt 6") != string::npos ||	// GT 6xx
					(renderer_str.find("geforce gtx 7") != string::npos &&	// GTX 7xx (!750)
					 (renderer_str.find("750") == string::npos ||
					  renderer_str.find("m") != string::npos)) ||
					renderer_str.find("geforce gts 7") != string::npos ||	// GTS 7xx
					renderer_str.find("geforce gt 7") != string::npos ||	// GT 7xx
					renderer_str.find("geforce titan") != string::npos) {	// Titan
				graphics_card = ext::GRAPHICS_CARD::GEFORCE_GK100;
			}
			else if(renderer_str.find("geforce gtx 750") != string::npos ||	// GTX 750
					renderer_str.find("geforce gtx 8") != string::npos ||	// GTX 8xx
					renderer_str.find("geforce gts 8") != string::npos ||	// GTS 8xx
					renderer_str.find("geforce gt 8") != string::npos) {	// GT 8xx
				graphics_card = ext::GRAPHICS_CARD::GEFORCE_GM100;
			}
		}
		else if(vendor == ext::GRAPHICS_CARD_VENDOR::ATI) {
			if(renderer_str.find("radeon hd 2") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::RADEON_HD2;
			}
			else if(renderer_str.find("radeon hd 3") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::RADEON_HD3;
			}
			else if(renderer_str.find("radeon hd 4") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::RADEON_HD4;
			}
			else if(renderer_str.find("radeon hd 5") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::RADEON_HD5;
			}
			else if(renderer_str.find("radeon hd 6") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::RADEON_HD6;
			}
			else if(renderer_str.find("radeon hd 7") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::RADEON_HD7;
			}
			else if(renderer_str.find("radeon hd 8") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::RADEON_HD8;
			}
		}
		else if(vendor == ext::GRAPHICS_CARD_VENDOR::POWERVR) {
			if(renderer_str.find("sgx 535") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::SGX_535;
			}
			else if(renderer_str.find("sgx 543") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::SGX_543;
			}
			else if(renderer_str.find("sgx 554") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::SGX_554;
			}
		}
		else if(vendor == ext::GRAPHICS_CARD_VENDOR::INTEL) {
			if(renderer_str.find("hd graphics 2500") != string::npos ||
			   renderer_str.find("hd graphics 4000") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::IVY_BRIDGE;
			}
			else if(renderer_str.find("hd graphics 5") != string::npos ||
					renderer_str.find("hd graphics 4200") != string::npos ||
					renderer_str.find("hd graphics 4400") != string::npos ||
					renderer_str.find("hd graphics 4600") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::HASWELL;
			}
		}
		else if(vendor == ext::GRAPHICS_CARD_VENDOR::APPLE) {
			if(renderer_str.find("a7") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::APPLE_A7;
			}
		}
		else {
			if(renderer_str.find("generic sm 4.0") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::GENERIC_SM4;
			}
			else if(renderer_str.find("generic sm 5.0") != string::npos) {
				graphics_card = ext::GRAPHICS_CARD::GENERIC_SM5;
			}
		}

		log_debug("your graphics card has been recognized as \"%s\"!", GRAPHICS_CARD_STR[(unsigned int)graphics_card]);
	}
	
	if(imode == engine::INIT_MODE::GRAPHICAL) {
		log_debug("opengl version: %s", cstr_from_gl_version(opengl_version));
		log_debug("glsl version: GLSL %s", cstr_from_glsl_version(glsl_version));
#if !defined(FLOOR_IOS)
		log_debug("shader model: %s", shader_support ? (shader_model_5_0_support ? "5.0" : "4.0") : "none");
#endif
	}
	
	// TODO: abort if no opengl (3.2) or glsl (1.50) support!
}

/*! delete everything
 */
ext::~ext() {
}

/*! returns true if the extension (ext_name) is supported by the graphics adapter
 *  @param ext_name the extensions name we want to look for if it's supported
 */
bool ext::is_ext_supported(const string& ext_name) {
	if(supported_extensions.count(ext_name) > 0) {
		return true;
	}
	return false;
}

/*! returns true if the opengl version specified by major and minor is available (opengl major.minor.xxxx)
 *  @param ext_name the extensions name we want to look for if it's supported
 */
bool ext::is_gl_version(unsigned int major, unsigned int minor) {
	const char* version = (const char*)glGetString(GL_VERSION);
	if((unsigned int)(version[0] - '0') > major) return true;
	else if((unsigned int)(version[0] - '0') == major && (unsigned int)(version[2] - '0') >= minor) return true;
	return false;
}

/*! returns the graphic card vendor
 */
ext::GRAPHICS_CARD_VENDOR ext::get_vendor() {
	return ext::vendor;
}

/*! returns the graphic card
 */
ext::GRAPHICS_CARD ext::get_graphics_card() {
	return ext::graphics_card;
}

/*! returns true if glsl shaders are supported
 */
bool ext::is_shader_support() {
	return ext::shader_support;
}

/*! returns true if shader model 5.0 is supported
 */
bool ext::is_shader_model_5_0_support() {
	return ext::shader_model_5_0_support;
}

/*! returns true if anisotropic filtering is supported
 */
bool ext::is_anisotropic_filtering_support() {
	return ext::anisotropic_filtering_support;
}

/*! returns max supported anisotropic filtering
 */
unsigned int ext::get_max_anisotropic_filtering() {
	return ext::max_anisotropic_filtering;
}

/*! returns max supported texture size
 */
unsigned int ext::get_max_texture_size() {
	return ext::max_texture_size;
}

/*! returns true if framebuffer object multisample coverage is supported
 */
bool ext::is_fbo_multisample_coverage_support() {
	return ext::fbo_multisample_coverage_support;
}

/*! returns max (fragment program/shader) texture image units
 */
unsigned int ext::get_max_texture_image_units() {
	return ext::max_texture_image_units;
}

/*! returns max samples (multisample)
 */
unsigned int ext::get_max_samples() {
	return ext::max_samples;
}

/*! returns max draw buffers
 */
unsigned int ext::get_max_draw_buffers() {
	return ext::max_draw_buffers;
}

/*! returns 
 */
bool ext::is_fbo_multisample_coverage_mode_support(unsigned int coverage_samples, unsigned int color_samples) {
	for(const auto& mode : multisample_coverage_modes) {
		if(mode.coverage_samples == (int)coverage_samples &&
		   mode.color_samples == (int)color_samples) {
			return true;
		}
	}
	return false;
}

const char* ext::cstr_from_glsl_version(const ext::GLSL_VERSION& version) const {
	switch(version) {
		case GLSL_VERSION::GLSL_ES_100: return "1.00";
		case GLSL_VERSION::GLSL_ES_300: return "3.00";
		case GLSL_VERSION::GLSL_ES_310: return "3.10";
		case GLSL_VERSION::GLSL_150: return "1.50";
		case GLSL_VERSION::GLSL_330: return "3.30";
		case GLSL_VERSION::GLSL_400: return "4.00";
		case GLSL_VERSION::GLSL_410: return "4.10";
		case GLSL_VERSION::GLSL_420: return "4.20";
		case GLSL_VERSION::GLSL_430: return "4.30";
		case GLSL_VERSION::GLSL_440: return "4.40";
		case GLSL_VERSION::GLSL_450: return "4.50";
		case GLSL_VERSION::GLSL_NO_VERSION: break;
	}
	return "<unknown>";
}

const char* ext::glsl_version_str_from_glsl_version(const ext::GLSL_VERSION& version) const {
	switch(version) {
		case GLSL_VERSION::GLSL_ES_100: return "100";
		case GLSL_VERSION::GLSL_ES_300: return "300";
		case GLSL_VERSION::GLSL_ES_310: return "310";
		case GLSL_VERSION::GLSL_150: return "150";
		case GLSL_VERSION::GLSL_330: return "330";
		case GLSL_VERSION::GLSL_400: return "400";
		case GLSL_VERSION::GLSL_410: return "410";
		case GLSL_VERSION::GLSL_420: return "420";
		case GLSL_VERSION::GLSL_430: return "430";
		case GLSL_VERSION::GLSL_440: return "440";
		case GLSL_VERSION::GLSL_450: return "450";
		case GLSL_VERSION::GLSL_NO_VERSION: break;
	}
	return "<unknown>";
}

ext::GLSL_VERSION ext::to_glsl_version(const size_t& major_version, const size_t& minor_version) const {
	switch(major_version) {
		case 1:
			switch(minor_version) {
				case 0: return GLSL_VERSION::GLSL_ES_100;
				case 50: return GLSL_VERSION::GLSL_150;
				default: break;
			}
			break;
		case 3:
			switch(minor_version) {
				case 0: return GLSL_VERSION::GLSL_ES_300;
				case 10: return GLSL_VERSION::GLSL_ES_310;
				case 30: return GLSL_VERSION::GLSL_330;
				default: break;
			}
			break;
		case 4:
			switch(minor_version) {
				case 0: return GLSL_VERSION::GLSL_400;
				case 10: return GLSL_VERSION::GLSL_410;
				case 20: return GLSL_VERSION::GLSL_420;
				case 30: return GLSL_VERSION::GLSL_430;
				case 40: return GLSL_VERSION::GLSL_440;
				case 50: return GLSL_VERSION::GLSL_450;
				default: break;
			}
			break;
		default:
			break;
	}
	
	log_error("invalid glsl version %d.%d!", major_version, minor_version);
	return GLSL_VERSION::GLSL_NO_VERSION;
}

ext::GLSL_VERSION ext::to_glsl_version(const size_t& version) const {
	switch(version) {
		case 100: return GLSL_VERSION::GLSL_ES_100;
		case 300: return GLSL_VERSION::GLSL_ES_300;
		case 310: return GLSL_VERSION::GLSL_ES_310;
		case 150: return GLSL_VERSION::GLSL_150;
		case 330: return GLSL_VERSION::GLSL_330;
		case 400: return GLSL_VERSION::GLSL_400;
		case 410: return GLSL_VERSION::GLSL_410;
		case 420: return GLSL_VERSION::GLSL_420;
		case 430: return GLSL_VERSION::GLSL_430;
		case 440: return GLSL_VERSION::GLSL_440;
		case 450: return GLSL_VERSION::GLSL_450;
	}
	
	log_error("invalid glsl version %d!", version);
	return GLSL_VERSION::GLSL_NO_VERSION;
}

const char* ext::cstr_from_gl_version(const ext::OPENGL_VERSION& version) const {
	switch(version) {
		case OPENGL_VERSION::OPENGL_ES_2_0: return "OpenGL ES 2.0";
		case OPENGL_VERSION::OPENGL_ES_3_0: return "OpenGL ES 3.0";
		case OPENGL_VERSION::OPENGL_ES_3_1: return "OpenGL ES 3.1";
		case OPENGL_VERSION::OPENGL_3_0: return "OpenGL 3.0";
		case OPENGL_VERSION::OPENGL_3_1: return "OpenGL 3.1";
		case OPENGL_VERSION::OPENGL_3_2: return "OpenGL 3.2";
		case OPENGL_VERSION::OPENGL_3_3: return "OpenGL 3.3";
		case OPENGL_VERSION::OPENGL_4_0: return "OpenGL 4.0";
		case OPENGL_VERSION::OPENGL_4_1: return "OpenGL 4.1";
		case OPENGL_VERSION::OPENGL_4_2: return "OpenGL 4.2";
		case OPENGL_VERSION::OPENGL_4_3: return "OpenGL 4.3";
		case OPENGL_VERSION::OPENGL_4_4: return "OpenGL 4.4";
		case OPENGL_VERSION::OPENGL_4_5: return "OpenGL 4.5";
		case OPENGL_VERSION::OPENGL_UNKNOWN : break;
	}
	return "<unknown>";
}
