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

#include "extensions.h"

// i won't add quadro or firegl cards here, since their naming scheme is inscrutable.
// users will have to use the "force profile" option in the config.xml (set to the
// appropriate geforce or radeon card) or simply use the "Generic SM x.0" profile.
// i won't add older cards here, b/c they are incapable of doing shaders and don't
// need any specific render paths which aren't already used via standard extension
// checking. i also won't add cards of other vendors, since i don't own any or could
// otherwise check what they are capable of. they are most certainly unimportant
// anyways and can simply use the generic profile.
const char* ext::GRAPHIC_CARD_STR[] = {
	"Unknown",
	"Generic SM 4.0",
	"Generic SM 5.0",
	"GeForce 8",
	"GeForce 9",
	"GeForce GT200",
	"GeForce GF100",
	"GeForce GK100",
	"Radeon HD2",
	"Radeon HD3",
	"Radeon HD4",
	"Radeon HD5",
	"Radeon HD6",
	"Radeon HD7",
	"PowerVR SGX535",
	"PowerVR SGX543",
};

const char* ext::GRAPHIC_CARD_VENDOR_DEFINE_STR[] = {
	"GCV_UNKNOWN",
	"GCV_NVIDIA",
	"GCV_ATI",
	"GCV_POWERVR",
};

const char* ext::GRAPHIC_CARD_DEFINE_STR[] = {
	"GC_UNKNOWN",
	"GC_GENERIC_SM_4_0",
	"GC_GENERIC_SM_5_0",
	"GC_GEFORCE_8",
	"GC_GEFORCE_9",
	"GC_GEFORCE_GT200",
	"GC_GEFORCE_GF100",
	"GC_GEFORCE_GK100",
	"GC_RADEON_HD2",
	"GC_RADEON_HD3",
	"GC_RADEON_HD4",
	"GC_RADEON_HD5",
	"GC_RADEON_HD6",
	"GC_RADEON_HD7",
	"GC_SGX_535",
	"GC_SGX_543",
};

/*! create and initialize the extension class
 */
ext::ext(unsigned int imode, string* disabled_extensions_, string* force_device_, string* force_vendor_) {
	ext::mode = imode;
	ext::disabled_extensions = disabled_extensions_;
	ext::force_device = force_device_;
	ext::force_vendor = force_vendor_;
	
#if !defined(A2E_IOS)
	init_gl_funcs();
#endif
	
	// get supported extensions
#if !defined(A2E_IOS)
	GLint ext_count = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &ext_count);
	for(int i = 0; i < ext_count; i++) {
		supported_extensions.insert((const char*)glGetStringi(GL_EXTENSIONS, i));
	}
#else
	const string exts = (const char*)glGetString(GL_EXTENSIONS);
	for(size_t pos = 0; (pos = exts.find("GL_", pos)) != string::npos; pos++) {
		supported_extensions.insert(exts.substr(pos, exts.find(" ", pos) - pos));
	}
#endif
	
	// get opengl and glsl version
	const string str_gl_version = (const char*)glGetString(GL_VERSION);
	const string str_glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	
	opengl_version = OPENGL_UNKNOWN;
#if !defined(A2E_IOS)
	switch(str_gl_version[0]) {
		case '0':
		case '1':
		case '2':
			opengl_version = OPENGL_UNKNOWN;
			break;
		case '3':
			opengl_version = OPENGL_3_0;
			switch(str_gl_version[2]) {
				case '0': break;
				case '1':
					opengl_version = OPENGL_3_1;
					break;
				case '2':
					opengl_version = OPENGL_3_2;
					break;
				case '3':
				default:
					opengl_version = OPENGL_3_3;
					break;
			}
			break;
		case '4':
			opengl_version = OPENGL_4_0;
			switch(str_gl_version[2]) {
				case '0': break;
				case '1':
					opengl_version = OPENGL_4_1;
					break;
				case '2':
				default:
					opengl_version = OPENGL_4_2;
					break;
			}
			break;
		default:
			// default to highest version
			opengl_version = OPENGL_4_2;
			break;
	}
#else
	const string str_gles_version = str_gl_version.substr(str_gl_version.find("OpenGL ES ")+10,
														  str_gl_version.length()-10);
	switch(str_gles_version[0]) {
		case '0':
		case '1':
			opengl_version = OPENGL_UNKNOWN;
			break;
		case '2':
			opengl_version = OPENGL_ES_2_0;
			break;
		default:
			// default to highest version
			opengl_version = OPENGL_ES_2_0;
			break;
	}
#endif
	
	glsl_version = GLSL_NO_VERSION;
#if !defined(A2E_IOS)
	switch(str_glsl_version[0]) {
		case '1':
			switch(str_glsl_version[2]) {
				case '5': glsl_version = GLSL_150; break;
				default: break;
			}
			break;
		case '3':
			switch(str_glsl_version[2]) {
				case '3':
				default:
					glsl_version = GLSL_330;
					break;
			}
			break;
		case '4':
			switch(str_glsl_version[2]) {
				case '0': glsl_version = GLSL_400; break;
				case '1': glsl_version = GLSL_410; break;
				case '2':
				default:
					glsl_version = GLSL_420;
					break;
			}
			break;
		default:
			// default to highest version
			glsl_version = GLSL_420;
			break;
	}
#else
	const string str_glsles_version = str_glsl_version.substr(str_glsl_version.find("OpenGL ES GLSL ES ")+18,
															  str_glsl_version.length()-18);
	switch(str_glsles_version[0]) {
		case '1':
			glsl_version = GLSL_ES_100;
			break;
		default:
			// default to highest version
			glsl_version = GLSL_ES_100;
			break;
	}
#endif

	// set all flags to false beforehand
	shader_support = (glsl_version >= GLSL_ES_100);
	shader_model_5_0_support = false;
	anisotropic_filtering_support = false;
	fbo_multisample_coverage_support = false;

	max_anisotropic_filtering = 0;
	max_texture_size = 0;
	max_texture_image_units = 0;
	max_samples = 0;
	max_multisample_coverage_modes = 0;
	multisample_coverage_modes = nullptr;
	max_draw_buffers = 0;
	
	if(imode == 0) {
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
		if(vendor_str.find("nvidia") != string::npos) vendor = ext::GCV_NVIDIA;
		// imagin(ati)on needs to be checked first ...
		else if(vendor_str.find("imagination") != string::npos) vendor = ext::GCV_POWERVR;
		else if(vendor_str.find("ati") != string::npos) vendor = ext::GCV_ATI;
		else vendor = ext::GCV_UNKNOWN;
	}
	else {
		vendor = ext::GCV_UNKNOWN;
		graphic_card = ext::GC_UNKNOWN;
	}
	
	if(imode == 0) {
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&max_texture_size);
	}

	// check extension support and set flags accordingly
	const struct {
		bool* flag;
		const char* exts[4];
	} ext_checks[] = {
		{ &fbo_multisample_coverage_support, { "GL_NV_framebuffer_multisample_coverage" } },
		{ &anisotropic_filtering_support, { "GL_EXT_texture_filter_anisotropic" } },
		{ &shader_model_5_0_support, { "GL_ARB_gpu_shader5" } },
	};
	
	for(const auto& check : ext_checks) {
		if(*check.flag == true) continue; // already supported by another extension
		*check.flag = true;
		for(const auto& ext_str : check.exts) {
			if(ext_str == nullptr) break;
			if(!is_ext_supported(ext_str)) {
				*check.flag = false;
				if(imode == 0) {
					a2e_msg("your graphic device doesn't support '%s'!", ext_str);
				}
				// don't break here, but rather print all extensions that aren't supported
			}
		}
	}

	//////
	if(!shader_model_5_0_support && glsl_version >= GLSL_400) {
		shader_model_5_0_support = true;
	}

	// get max multi-sampling samples
#if !defined(A2E_IOS)
	if(fbo_multisample_coverage_support) {
		glGetIntegerv(GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV, (GLint*)&max_multisample_coverage_modes);
		multisample_coverage_modes = new ext::NV_MULTISAMPLE_COVERAGE_MODE[max_multisample_coverage_modes];
		glGetIntegerv(GL_MULTISAMPLE_COVERAGE_MODES_NV, (GLint*)multisample_coverage_modes);
	}
#endif
	if(imode == 0) glGetIntegerv(GL_MAX_SAMPLES, (GLint*)&max_samples);
	
	// get max vertex textures
	GLint vtf = 0;
	if(imode == 0) glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &vtf);
	if(vtf > 0) {
		a2e_debug("supported vertex textures: %i", vtf);
	}
	else {
		if(imode == 0) a2e_error("your graphic device doesn't support 'Vertex Texture Fetching'!");
	}
	
	// get max anisotropic filtering
	if(anisotropic_filtering_support) {
		max_anisotropic_filtering = 1;
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, (GLint*)(&max_anisotropic_filtering));
	}
	
	// get max textures
	if(imode == 0) glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*)&(ext::max_texture_image_units));
	
	// get max draw buffers
#if !defined(A2E_IOS)
	if(imode == 0) glGetIntegerv(GL_MAX_DRAW_BUFFERS, (GLint*)&(ext::max_draw_buffers));
#else
	// TODO: no support on iOS (yet), workaround?
	max_draw_buffers = 0;
#endif

	// graphic card handling
	if(imode == 0) {
		graphic_card = (shader_support ? (shader_model_5_0_support ?
			ext::GC_GENERIC_SM5 : ext::GC_GENERIC_SM4) : GC_UNKNOWN);

		// get renderer string and make it lower case
		const char* gl_renderer_str = (const char*)glGetString(GL_RENDERER);
		if(gl_renderer_str == nullptr) {
			a2e_error("gl renderer str is nullptr - invalid opengl context?");
		}
		string renderer_str = (*force_device == "") ? (gl_renderer_str == nullptr ? "" : gl_renderer_str) : force_device->c_str();
		core::str_to_lower_inplace(renderer_str);
		if(vendor == ext::GCV_NVIDIA) {
			if(renderer_str.find("geforce 8") != string::npos) {
				graphic_card = ext::GC_GEFORCE_8;
			}
			// yes, these are all actually geforce 9 cards ... and thank you nvidia for this totally fucked up naming scheme ...
			else if(renderer_str.find("geforce 9") != string::npos ||		// 9xxx(M)
					renderer_str.find("geforce 1") != string::npos ||			// 1xx(M)
					renderer_str.find("geforce g 1") != string::npos ||		// G 1xx(M)
					renderer_str.find("geforce g1") != string::npos ||		// G1xx(M)
					renderer_str.find("geforce gt 1") != string::npos ||		// GT 1xx(M)
					renderer_str.find("geforce gts 1") != string::npos ||	// GTS 1xx(M)
					(renderer_str.find("geforce gts 2") != string::npos && renderer_str.find("m") == string::npos) || // GTS 2xx (!M)
					(renderer_str.find("geforce gtx 2") != string::npos && renderer_str.find("m") != string::npos)) { // GTX 2xxM
				graphic_card = ext::GC_GEFORCE_9;
			}
			else if(renderer_str.find("geforce 2") != string::npos ||		// 2xx(M)
					renderer_str.find("geforce 3") != string::npos ||			// 3xx(M)
					renderer_str.find("geforce g 2") != string::npos ||		// G 2xx(M)
					renderer_str.find("geforce g2") != string::npos ||		// G2xx(M)
					renderer_str.find("geforce gt 2") != string::npos ||		// GT 2xx(M)
					renderer_str.find("geforce gts 2") != string::npos ||	// GTS 2xx(M)
					renderer_str.find("geforce gtx 2") != string::npos ||	// GTX 2xx
					renderer_str.find("geforce gt 3") != string::npos ||		// GT 3xx(M)
					renderer_str.find("geforce gts 3") != string::npos) {	// GTS 3xx(M)
				graphic_card = ext::GC_GEFORCE_GT200;
			}
			else if(renderer_str.find("geforce gtx 4") != string::npos ||	// GTX 4xx
					renderer_str.find("geforce gts 4") != string::npos ||	// GTS 4xx
					renderer_str.find("geforce gt 4") != string::npos ||		// GT 4xx
					renderer_str.find("geforce gtx 5") != string::npos ||	// GTX 5xx
					renderer_str.find("geforce gts 5") != string::npos ||	// GTS 5xx
					renderer_str.find("geforce gt 5") != string::npos ||		// GT 5xx
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
				graphic_card = ext::GC_GEFORCE_GF100;
			}
			else if(renderer_str.find("geforce gtx 6") != string::npos ||	// GTX 6xx
					renderer_str.find("geforce gts 6") != string::npos ||	// GTS 6xx
					renderer_str.find("geforce gt 6") != string::npos) {		// GT 6xx
				graphic_card = ext::GC_GEFORCE_GK100;
			}
		}
		else if(vendor == ext::GCV_ATI) {
			if(renderer_str.find("radeon hd 2") != string::npos) {
				graphic_card = ext::GC_RADEON_HD2;
			}
			else if(renderer_str.find("radeon hd 3") != string::npos) {
				graphic_card = ext::GC_RADEON_HD3;
			}
			else if(renderer_str.find("radeon hd 4") != string::npos) {
				graphic_card = ext::GC_RADEON_HD4;
			}
			else if(renderer_str.find("radeon hd 5") != string::npos) {
				graphic_card = ext::GC_RADEON_HD5;
			}
			else if(renderer_str.find("radeon hd 6") != string::npos) {
				graphic_card = ext::GC_RADEON_HD6;
			}
			else if(renderer_str.find("radeon hd 7") != string::npos) {
				graphic_card = ext::GC_RADEON_HD7;
			}
		}
		else if(vendor == ext::GCV_POWERVR) {
			if(renderer_str.find("sgx 535") != string::npos) {
				graphic_card = ext::GC_SGX_535;
			}
			else if(renderer_str.find("sgx 543") != string::npos) {
				graphic_card = ext::GC_SGX_543;
			}
		}
		else {
			if(renderer_str.find("generic sm 4.0") != string::npos) {
				graphic_card = ext::GC_GENERIC_SM4;
			}
			else if(renderer_str.find("generic sm 5.0") != string::npos) {
				graphic_card = ext::GC_GENERIC_SM5;
			}
		}

		a2e_debug("your graphic card has been recognized as \"%s\"!", GRAPHIC_CARD_STR[graphic_card]);
	}
	
	if(imode == 0) {
		a2e_debug("opengl version: %s", cstr_from_gl_version(opengl_version));
		a2e_debug("glsl version: GLSL %s", cstr_from_glsl_version(glsl_version));
		a2e_debug("shader model: %s", shader_support ? (shader_model_5_0_support ? "5.0" : "4.0") : "none");
	}
	
	// TODO: abort if no opengl (3.2) or glsl (1.50) support!
}

/*! delete everything
 */
ext::~ext() {
	if(multisample_coverage_modes != nullptr) delete [] multisample_coverage_modes;
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
ext::GRAPHIC_CARD_VENDOR ext::get_vendor() {
	return ext::vendor;
}

/*! returns the graphic card
 */
ext::GRAPHIC_CARD ext::get_graphic_card() {
	return ext::graphic_card;
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
	for(unsigned int i = 0; i < max_multisample_coverage_modes; i++) {
		if(multisample_coverage_modes[i].coverage_samples == (int)coverage_samples && multisample_coverage_modes[i].color_samples == (int)color_samples) return true;
	}
	return false;
}

const char* ext::cstr_from_glsl_version(const ext::GLSL_VERSION& version) const {
	switch(version) {
		case GLSL_ES_100: return "1.00";
		case GLSL_150: return "1.50";
		case GLSL_330: return "3.30";
		case GLSL_400: return "4.00";
		case GLSL_410: return "4.10";
		case GLSL_420: return "4.20";
		default:
			break;
	}
	return "<unknown>";
}

const char* ext::glsl_version_str_from_glsl_version(const ext::GLSL_VERSION& version) const {
	switch(version) {
		case GLSL_ES_100: return "100";
		case GLSL_150: return "150";
		case GLSL_330: return "330";
		case GLSL_400: return "400";
		case GLSL_410: return "410";
		case GLSL_420: return "420";
		default: break;
	}
	return "<unknown>";
}

ext::GLSL_VERSION ext::to_glsl_version(const size_t& major_version, const size_t& minor_version) const {
	switch(major_version) {
		case 1:
			switch(minor_version) {
				case 0: return GLSL_ES_100;
				case 50: return GLSL_150;
				default: break;
			}
			break;
		case 3:
			switch(minor_version) {
				case 30: return GLSL_330;
				default: break;
			}
			break;
		case 4:
			switch(minor_version) {
				case 0: return GLSL_400;
				case 10: return GLSL_410;
				case 20: return GLSL_420;
				default: break;
			}
			break;
		default:
			break;
	}
	
	a2e_error("invalid glsl version %d.%d!", major_version, minor_version);
	return GLSL_NO_VERSION;
}

ext::GLSL_VERSION ext::to_glsl_version(const size_t& version) const {
	switch(version) {
		case 100: return GLSL_ES_100;
		case 150: return GLSL_150;
		case 330: return GLSL_330;
		case 400: return GLSL_400;
		case 410: return GLSL_410;
		case 420: return GLSL_420;
	}
	
	a2e_error("invalid glsl version %d!", version);
	return GLSL_NO_VERSION;
}

const char* ext::cstr_from_gl_version(const ext::OPENGL_VERSION& version) const {
	switch(version) {
		case OPENGL_ES_2_0: return "OpenGL ES 2.0";
		case OPENGL_3_0: return "OpenGL 3.0";
		case OPENGL_3_1: return "OpenGL 3.1";
		case OPENGL_3_2: return "OpenGL 3.2";
		case OPENGL_3_3: return "OpenGL 3.3";
		case OPENGL_4_0: return "OpenGL 4.0";
		case OPENGL_4_1: return "OpenGL 4.1";
		case OPENGL_4_2: return "OpenGL 4.2";
		default: break;
	}
	return "<unknown>";
}
