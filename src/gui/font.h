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

#ifndef __A2E_FONT_H__
#define __A2E_FONT_H__

#include "global.h"
#include "gui/event.h"

/*! @class font
 *  @brief stores a font and can be used for drawing
 */

class engine;
class shader;
class font_manager;
typedef struct FT_FaceRec_* FT_Face;
class shader_gl3;
typedef shared_ptr<shader_gl3> gl3shader;
enum class EVENT_TYPE : unsigned int;
struct event_object;
class A2E_API font {
public:
	//! single font file or font collection
	font(engine* e, font_manager* fm, const string& filename);
	//! multiple font files (note: only the first of each style will be used)
	font(engine* e, font_manager* fm, const vector<string> filenames);
	~font();
	
	// draw functions
	void draw(const string& text, const float2& position, const float4 color = float4(1.0f));
	void draw_cached(const string& text, const float2& position, const float4 color = float4(1.0f)) const;
	void draw_cached(const GLuint& ubo, const size_t& character_count, const float2& position, const float4 color = float4(1.0f)) const;
	
	// style functions
	const vector<string> get_available_styles() const;
	const unsigned int& get_size() const;
	const unsigned int& get_display_size() const;
	
	float compute_advance(const string& str, const unsigned int component = 0) const;
	float compute_advance(const vector<unsigned int>& unicode_str, const unsigned int component = 0) const;
	vector<float2> compute_advance_map(const string& str, const unsigned int component = 0) const;
	vector<float2> compute_advance_map(const vector<unsigned int>& unicode_str, const unsigned int component = 0) const;
	
	//! http://en.wikipedia.org/wiki/Basic_Multilingual_Plane#Basic_Multilingual_Plane
	enum class BMP_BLOCK : unsigned int {
		BASIC_LATIN				= 0x0000007F, // NOTE: will be cached automatically (0000 - 007F)
		LATIN_1_SUPPLEMENT		= 0x008000FF, // NOTE: will be cached automatically (0080 - 00FF)
		LATIN_EXTENDED_A		= 0x0100017F, // 0100 - 017F
		LATIN_EXTENDED_B		= 0x0180024F, // 0180 - 024F
		LATIN_IPA_EXTENSIONS	= 0x025002AF, // 0250 - 02AF
		GREEK					= 0x037003FF, // 0370 - 03FF
		CYRILLIC				= 0x040004FF, // 0400 - 04FF
		CYRILLIC_SUPPLEMENT		= 0x0500052F, // 0500 - 052F
		ARMENIAN				= 0x0530058F, // 0530 - 058F
		HEBREW					= 0x059005FF, // 0590 - 05FF
		ARABIC					= 0x060006FF, // 0600 - 06FF
		ARABIC_SUPPLEMENT		= 0x0750077F, // 0750 - 077F
		HIRAGANA				= 0x3040309F, // 3040 - 309F
		KATAKANA				= 0x30A030FF  // 30A0 - 30FF
	};
	//! shortcut for common blocks
	void cache(const BMP_BLOCK block);
	//! this must be utf-8 encoded
	void cache(const string& characters);
	//! should be within 0x0 - 0x10FFFF
	void cache(const unsigned int& start_code, const unsigned int& end_code);
	//! <<ubo, character_count>, extent>, note: ubo must be destroyed/managed manually!
	typedef pair<uint2, float2> text_cache;
	text_cache cache_text(const string& text, const GLuint existing_ubo = 0);
	static void destroy_text_cache(text_cache& cached_text);
	
	bool is_cached(const unsigned int& code) const;
	
	// TODO: clear cache
	//void clear_cache();
	
	// texture cache info
	static constexpr unsigned int font_texture_size = 1024;
	
	// unicode -> texture index
	struct glyph_data {
		const unsigned int tex_index;
		const int4 layout;
		const int2 size;
	};
	
protected:
	engine* e = nullptr;
	shader* s = nullptr;
	font_manager* fm = nullptr;
	const vector<string> filenames;
	string font_name = "NONE";
	
	// style -> ft face
	unordered_map<string, FT_Face> faces;
	bool add_face(const string& style, FT_Face face);
	
	// style -> (unicode -> glyph data)
	unordered_map<string, unordered_map<unsigned int, glyph_data>> glyph_map;
	unsigned int font_size = 10;
	unsigned int display_font_size = font_size;
	unsigned int glyphs_per_line = 0;
	unsigned int glyphs_per_layer = 0;
	
	void recreate_texture_array(const size_t& layers);
	GLuint tex_array = 0;
	size_t tex_array_layers = 0;
	GLuint glyph_vbo = 0;
	
	pair<vector<uint2>, float2> create_text_ubo_data(const string& text, std::function<void(unsigned int)> cache_fnc = [](unsigned int){}) const;
	GLuint text_ubo = 0;
		
	gl3shader font_shd;
	void reload_shaders();
	event::handler shader_reload_fnctr;
	bool shader_reload_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	
	// size functions (TODO: will be made public when dynamic size changes are supported)
	void set_size(const unsigned int& size);
	
	//
	float2 text_stepper(const string& str,
						std::function<void(unsigned int, const glyph_data&, const float2&, const float2&)> fnc = [](unsigned int, const glyph_data&, const float2&, const float2&){},
						std::function<void(unsigned int, const float2&, const float&)> line_break_fnc = [](unsigned int, const float2&, const float&){},
						std::function<void(unsigned int)> cache_fnc = [](unsigned int){}) const;
	float2 text_stepper(const vector<unsigned int>& unicode_str,
						std::function<void(unsigned int, const glyph_data&, const float2&, const float2&)> fnc = [](unsigned int, const glyph_data&, const float2&, const float2&){},
						std::function<void(unsigned int, const float2&, const float&)> line_break_fnc = [](unsigned int, const float2&, const float&){},
						std::function<void(unsigned int)> cache_fnc = [](unsigned int){}) const;
	
};

#endif
