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

#include "font.h"
#include "font_manager.h"
#include "core/vector2.h"
#include "core/vector3.h"
#include "gui/unicode.h"
#include "gui/event_objects.h"
#include "engine.h"
#include "rendering/shader.h"
#include "rendering/renderer/gl3/shader_gl3.h"
#include <numeric>

#include <ft2build.h>
#include FT_FREETYPE_H

constexpr unsigned int font::font_texture_size;

font::font(engine* e_, font_manager* fm_, const string& filename_) : font(e_, fm_, vector<string> { filename_ }) {
}

font::font(engine* e_, font_manager* fm_, const vector<string> filenames_) :
e(e_), s(e_->get_shader()), fm(fm_), filenames(filenames_),
shader_reload_fnctr(this, &font::shader_reload_handler)
{
	set_size(10);
	
	// load fonts
	for(const auto& filename : filenames) {
		// load first face of file
		FT_Face first_face = nullptr;
		if(FT_New_Face(fm->get_ft_library(), filename.c_str(), 0, &first_face) != 0) {
			a2e_error("couldn't load font %s!", filename);
			return;
		}
		if(font_name == "NONE") font_name = first_face->family_name;
		const size_t num_faces = first_face->num_faces;
		
		// add if style doesn't exist yet
		if(!add_face(first_face->style_name, first_face)) {
			first_face = nullptr;
		}
		
		// load other faces if there are any and set properties
		for(size_t face_index = 0; face_index < num_faces; face_index++) {
			FT_Face face = nullptr;
			if(face_index > 0) {
				if(FT_New_Face(fm->get_ft_library(), filename.c_str(), face_index, &face) != 0) {
					a2e_error("couldn't load face #%u for font %s!", face_index, filename);
					return;
				}
				// add if style doesn't exist yet
				if(!add_face(face->style_name, face)) {
					face = nullptr;
				}
			}
			else {
				face = first_face;
			}
			
			if(face != nullptr) {
				if(FT_Set_Char_Size(face, 0, font_size * 64, 0, (FT_UInt)e->get_dpi()) != 0) {
					a2e_error("couldn't set char size for face #%u in font %s!", face_index, filename);
					return;
				}
			}
		}
	}
	
	// make sure we have a font for each style (if not, use another one as fallback)
	if(faces.empty()) {
		a2e_error("couldn't load any faces for font %s!", font_name);
		return;
	}
	
	// TODO: use emplace (w/o make_pair) instead of insert when gcc supports it
	if(faces.find("Regular") == faces.end()) {
		// use any since there is no real fallback
		faces.insert(make_pair("Regular", faces.cbegin()->second));
	}
	if(faces.find("Bold Italic") == faces.end()) {
		if(faces.find("Bold") != faces.end()) {
			faces.insert(make_pair("Bold Italic", faces.find("Bold")->second));
		}
		if(faces.find("Italic") != faces.end()) {
			faces.insert(make_pair("Bold Italic", faces.find("Italic")->second));
		}
		else {
			faces.insert(make_pair("Bold Italic", faces.find("Regular")->second));
		}
	}
	if(faces.find("Italic") == faces.end()) {
		faces.insert(make_pair("Italic", faces.find("Regular")->second));
	}
	if(faces.find("Bold") == faces.end()) {
		faces.insert(make_pair("Bold", faces.find("Regular")->second));
	}
	
	// cache most used glyphs
	cache(BMP_BLOCK::BASIC_LATIN);
	cache(BMP_BLOCK::LATIN_1_SUPPLEMENT);
	
	// create necessary glyph vbo and ubo
	glGenBuffers(1, &glyph_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);
	const float2 glyph_quad[] {
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f),
	};
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float2), &glyph_quad[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &text_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, text_ubo);
	glBufferData(GL_UNIFORM_BUFFER, 65536, nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	//
	e->get_event()->add_internal_event_handler(shader_reload_fnctr, EVENT_TYPE::SHADER_RELOAD);
	reload_shaders();
}

font::~font() {
	// different styles may point to the same FT_Font -> create a set
	set<FT_Face> ft_faces;
	for(const auto& face : faces) {
		// TODO: -> emplace
		ft_faces.insert(face.second);
	}
	for(const auto& face : ft_faces) {
		if(FT_Done_Face(face) != 0) {
			a2e_error("failed to free face for font %s!", font_name);
		}
	}
	
	if(glIsBuffer(glyph_vbo)) glDeleteBuffers(1, &glyph_vbo);
	if(glIsBuffer(text_ubo)) glDeleteBuffers(1, &text_ubo);
	if(glIsTexture(tex_array)) glDeleteTextures(1, &tex_array);
	
	e->get_event()->remove_event_handler(shader_reload_fnctr);
}

bool font::add_face(const string& style, FT_Face face) {
	// style remapping
	auto map_style = [](string style_str) -> string {
		static const unordered_map<string, string> style_remapping {
			{ "Roman", "Regular" },
			{ "Book", "Regular" },
			{ "Oblique", "Italic" },
			{ "Bold Oblique", "Bold Italic" },
			{ "R", "Regular" },
			{ "I", "Italic" },
			{ "O", "Italic" },
			{ "B", "Bold" },
			{ "BI", "Bold Italic" },
			{ "BO", "Bold Italic" },
			{ "Medium", "Regular" },
			{ "M", "Regular" },
			{ "Medium Italic", "Italic" },
			{ "MI", "Italic" },
			{ "BoldItalic", "Bold Italic" },
			{ "BoldOblique", "Bold Italic" },
		};
		const auto iter = style_remapping.find(style_str);
		return (iter == style_remapping.end() ? style_str : iter->second);
	};
	
	const string style_name(map_style(style));
	if(faces.find(style_name) == faces.end()) {
		// TODO: i'm repeating myself ... -> emplace
		faces.insert(make_pair(style_name, face));
		return true;
	}
	
	FT_Done_Face(face);
	return false;
}

void font::cache(const BMP_BLOCK block) {
	cache(((unsigned int)block >> 16) & 0xFFFF, (unsigned int)block & 0xFFFF);
}

void font::cache(const string& characters) {
	const vector<unsigned int> codes(unicode::utf8_to_unicode(characters));
	for(const auto& code : codes) {
		cache(code, code);
	}
}

void font::cache(const unsigned int& start_code, const unsigned int& end_code) {
	if(start_code > end_code) {
		a2e_error("start_code(%u) > end_code(%u)", start_code, end_code);
		return;
	}
	if(start_code > 0x10FFFF || end_code > 0x10FFFF) {
		a2e_error("invalid start_code(%u) or end_code(%u) - >0x10FFFF!", start_code, end_code);
		return;
	}
	
	// check if we need to expand the texture to fit all glyphs in
	const size_t new_glyph_count = (end_code - start_code + 1) * faces.size();
	const size_t cur_glyph_count = accumulate(cbegin(glyph_map), cend(glyph_map), 0,
											  [](const size_t& accu, const decltype(glyph_map)::value_type& elem) {
												  return accu + elem.second.size(); // TODO: when glyph_data will contain more than one glyph, this needs to be computed here
	});
	if((cur_glyph_count + new_glyph_count) > (tex_array_layers * glyphs_per_layer)) {
		recreate_texture_array(((cur_glyph_count + new_glyph_count) / glyphs_per_layer) + 1);
	}
	
	// render glyph bitmaps and copy them to the texture
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex_array);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // necessary, b/c we'll upload data that has no 4 or 2 byte alignment
	
	size_t glyph_counter = cur_glyph_count;
	for(const auto& face : faces) {
		auto& glyphs = glyph_map[face.first];
		FT_GlyphSlot slot = face.second->glyph;
		for(unsigned int code = start_code; code < end_code+1; code++) {
			if(glyphs.find(code) != glyphs.end()) {
				// glyph has already been cached
				continue;
			}
			
			if(FT_Load_Char(face.second, code, FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT | FT_LOAD_TARGET_LCD) != 0) {
				a2e_error("couldn't cache character %X!", code);
				continue;
			}
			
			//
			const unsigned int texture_index = (unsigned int)glyph_counter++;
			// TODO: -> emplace
			glyphs.insert(make_pair(code,
									glyph_data {
										texture_index,
										int4(slot->bitmap_left,
											 int(display_font_size) - slot->bitmap_top,
											 (int)slot->advance.x,
											 (int)slot->advance.y)
									}));
			const unsigned int layer = texture_index / glyphs_per_layer;
			const int3 offset((texture_index - (layer*glyphs_per_layer)) % glyphs_per_line,
							  (texture_index - (layer*glyphs_per_layer)) / glyphs_per_line,
							  layer);
			
			// ignore 0px writes ...
			if(slot->bitmap.width == 0 || slot->bitmap.rows == 0) continue;
			
			// copy glyph bitmap rows to a correctly sized bitmap (display_font_size^2 or max glyph size^2),
			// this is necessary, because of row padding "provided" by freetype
			const size_t glyph_mem_size(display_font_size * display_font_size * 3);
			unsigned char* glyph_buffer = new unsigned char[glyph_mem_size];
			memset(glyph_buffer, 0, glyph_mem_size);
			
			const size_t row_data_length(display_font_size * 3);
			for(size_t row = 0; row < (size_t)slot->bitmap.rows; row++) {
				memcpy(glyph_buffer + (row * row_data_length),
					   slot->bitmap.buffer + (row * slot->bitmap.pitch),
					   slot->bitmap.width);
			}
			
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
							offset.x * display_font_size, offset.y * display_font_size, offset.z,
							display_font_size, display_font_size, 1,
							GL_RGB, GL_UNSIGNED_BYTE, glyph_buffer);
			delete [] glyph_buffer;
		}
	}
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

uint2 font::cache_text(const string& text, const GLuint existing_ubo) {
	//
	GLuint ubo = existing_ubo;
	if(ubo == 0 || !glIsBuffer(ubo)) {
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, 65536, nullptr, GL_STATIC_DRAW);
	}
	else {
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	}
	
	// update ubo with text data
	const vector<uint2> ubo_data(create_text_ubo_data(text, [&](unsigned int code) {
		this->cache(code, code);
	}));
	
	glBufferSubData(GL_UNIFORM_BUFFER, 0, ubo_data.size() * sizeof(uint2), &ubo_data[0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	return uint2(ubo, (unsigned int)ubo_data.size());
}

void font::recreate_texture_array(const size_t& layers) {
	const size_t prev_layers(tex_array_layers);
	
	// this works around an intel and ati driver bug (an array must at least have 2 layers)
	tex_array_layers = std::max(layers, (size_t)2);
	
	const size_t layer_size(font_texture_size * font_texture_size * 3);
	const size_t tex_size(layer_size * tex_array_layers);
	unsigned char* tex_data = new unsigned char[tex_size];
	
	// check if we need to copy data from the old tex layers
	if(prev_layers != 0 && tex_array != 0) {
		glBindTexture(GL_TEXTURE_2D_ARRAY, tex_array);
		glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
		
		const size_t layer_diff(tex_array_layers - prev_layers);
		memset(tex_data + prev_layers * layer_size, 0, layer_diff * layer_size); // clear new layers
	}
	else {
		memset(tex_data, 0, tex_size);
	}
	
	//
	if(tex_array == 0) {
		glGenTextures(1, &tex_array);
	}
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex_array);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8,
				 font_texture_size, font_texture_size, (GLsizei)tex_array_layers,
				 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
	
	delete [] tex_data;
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void font::reload_shaders() {
	font_shd = s->get_gl3shader("FONT");
}

bool font::shader_reload_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(type == EVENT_TYPE::SHADER_RELOAD) {
		reload_shaders();
	}
	return true;
}

void font::draw(const string& text, const float2& position, const float4 color) {
	const auto ubo = cache_text(text, text_ubo);
	draw_cached(text_ubo, ubo.y, position, color);
}

void font::draw_cached(const string& text, const float2& position, const float4 color) const {
	// update ubo with text data
	const vector<uint2> ubo_data(create_text_ubo_data(text));
	
	glBindBuffer(GL_UNIFORM_BUFFER, text_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, ubo_data.size() * sizeof(uint2), &ubo_data[0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	draw_cached(text_ubo, ubo_data.size(), position, color);
}

vector<uint2> font::create_text_ubo_data(const string& text,
										 std::function<void (unsigned int)> cache_fnc) const {
	// replace control strings by control characters (easier to handle later on)
	static const struct {
		const string search;
		const string repl;
	} control_chars[] = {
		{ u8"<i>", u8"\u0001" },
		{ u8"</i>", u8"\u0002" },
		{ u8"<b>", u8"\u0003" },
		{ u8"</b>", u8"\u0004" },
	};
	string repl_text(text);
	for(const auto& cc : control_chars) {
		repl_text = core::find_and_replace(repl_text, cc.search, cc.repl);
	}
	
	// convert to utf-32
	const vector<unsigned int> codes(unicode::utf8_to_unicode(repl_text));
	
	vector<uint2> ubo_data;
	float2 origin(0.0f);
	string cur_style = "Regular";
	
	const decltype(glyph_map)::value_type::second_type& regular_map(glyph_map.find("Regular")->second);
	const decltype(glyph_map)::value_type::second_type& italic_map(glyph_map.find("Italic")->second);
	const decltype(glyph_map)::value_type::second_type& bold_map(glyph_map.find("Bold")->second);
	const decltype(glyph_map)::value_type::second_type& bold_italic_map(glyph_map.find("Bold Italic")->second);
	const decltype(glyph_map)::value_type::second_type* cur_style_map = &regular_map;
	
	static const unsigned int tab_multiplier = 4;
	static const float leading_multiplier = 1.125f;
	const unordered_map<string, unordered_map<unsigned int, unsigned int>> whitespace_sizes {
		{
			"Regular", {
				{ 0x0A, float(display_font_size) * leading_multiplier },
				{ 0x0D, float(display_font_size) * leading_multiplier },
				{ 0x09, (regular_map.find(0x20)->second.size.z >> 6) * tab_multiplier },
				{ 0x20, (regular_map.find(0x20)->second.size.z >> 6) },
			}
		},
		{
			"Italic", {
				{ 0x0A, float(display_font_size) * leading_multiplier },
				{ 0x0D, float(display_font_size) * leading_multiplier },
				{ 0x09, (italic_map.find(0x20)->second.size.z >> 6) * tab_multiplier },
				{ 0x20, (italic_map.find(0x20)->second.size.z >> 6) },
			}
		},
		{
			"Bold", {
				{ 0x0A, float(display_font_size) * leading_multiplier },
				{ 0x0D, float(display_font_size) * leading_multiplier },
				{ 0x09, (bold_map.find(0x20)->second.size.z >> 6) * tab_multiplier },
				{ 0x20, (bold_map.find(0x20)->second.size.z >> 6) },
			}
		},
		{
			"Bold Italic", {
				{ 0x0A, float(display_font_size) * leading_multiplier },
				{ 0x0D, float(display_font_size) * leading_multiplier },
				{ 0x09, (bold_italic_map.find(0x20)->second.size.z >> 6) * tab_multiplier },
				{ 0x20, (bold_italic_map.find(0x20)->second.size.z >> 6) },
			}
		},
	};
	const auto whitespace_size = [&](const unsigned int code) -> unsigned int {
		return whitespace_sizes.find(cur_style)->second.find(code)->second;
	};
	
	bool style_italic = false, style_bold = false;
	for(const auto& code : codes) {
		// handle control characters / whitespace
		switch(code) {
			case 0x01:
				style_italic = true;
				break;
			case 0x02:
				style_italic = false;
				break;
			case 0x03:
				style_bold = true;
				break;
			case 0x04:
				style_bold = false;
				break;
			case 0x0A:
			case 0x0D:
				origin.y += whitespace_size(0x0A);
				origin.x = 0.0f;
				continue;
			case 0x09:
				origin.x += whitespace_size(0x09);
				continue;
			default: break;
		}
		
		//
		switch(code) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04: {
				const size_t style_idx = size_t(style_italic) + size_t(style_bold)*2;
				cur_style = vector<string> { "Regular", "Italic", "Bold", "Bold Italic" }[style_idx];
				cur_style_map = vector<decltype(cur_style_map)> { &regular_map, &italic_map, &bold_map, &bold_italic_map }[style_idx];
				continue;
			}
			default: break;
		}
		
		auto iter = cur_style_map->find(code);
		if(iter == cur_style_map->end()) {
			// cache before we continue
			cache_fnc(code);
			
			// try again
			iter = cur_style_map->find(code);
			if(iter == cur_style_map->end()) {
				// if it hasn't been cached, ignore it
				origin.x += whitespace_size(0x20);
				continue;
			}
		}
		
		const int2 pos(origin.x + float(iter->second.size.x), origin.y + float(iter->second.size.y));
		origin.x += iter->second.size.z >> 6;
		origin.y += iter->second.size.w >> 6;
		ubo_data.emplace_back(iter->second.tex_index,
							  (unsigned int)(((short int)pos.x) & 0xFFFFu) +
							  (((unsigned int)(((short int)pos.y) & 0xFFFFu)) << 16u));
	}
	return ubo_data;
}

void font::draw_cached(const GLuint& ubo, const size_t& character_count, const float2& position, const float4 color) const {
	if(ubo == 0 || character_count == 0) return;
	
	// draw
	font_shd->use();
	font_shd->uniform("mvpm", matrix4f().translate(position.x, position.y, 0.0f) * *e->get_mvp_matrix());
	font_shd->uniform("glyph_count", uint2(glyphs_per_line, glyphs_per_line));
	font_shd->uniform("glyph_size", float2(display_font_size));
	font_shd->uniform("page_size", float2(font_texture_size));
	font_shd->uniform("font_color", color);
	font_shd->texture("font_texture", tex_array, GL_TEXTURE_2D_ARRAY);
	font_shd->block("text", ubo);
	font_shd->attribute_array("in_vertex", glyph_vbo, 2);
	
	// note: ubo data size / character_count (actual) != codes size / #characters in text (wanted)
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)character_count);
	
	font_shd->disable();
}

void font::set_size(const unsigned int& size) {
	font_size = size;
	// slightly weird, but it seems to work for misc dpi sizes
	display_font_size = (unsigned int)ceilf((float(font_size * e->get_dpi()) / 64.0f) * (72.0f / 64.0f));
	glyphs_per_line = font_texture_size / display_font_size;
	glyphs_per_layer = glyphs_per_line * glyphs_per_line;
}

const unsigned int& font::get_size() const {
	return font_size;
}

const unsigned int& font::get_display_size() const {
	return display_font_size;
}

const vector<string> font::get_available_styles() const {
	vector<string> ret;
	for(const auto& face : faces) {
		ret.emplace_back(face.first);
	}
	return ret;
}
