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


#include "a2e_shader.h"
#include "rendering/shader.h"
#include "core/xml.h"
#include <regex>

a2e_shader::a2e_shader(engine* eng) :
e(eng), f(e->get_file_io()), exts(e->get_ext()), x(e->get_xml()),
conditions({
	// add graphic card specific conditions
	{ ext::GRAPHICS_CARD_VENDOR_DEFINE_STR[(unsigned int)exts->get_vendor()], true },
	{ ext::GRAPHICS_CARD_DEFINE_STR[(unsigned int)exts->get_graphics_card()], true },
	{ "SM50_SUPPORT", exts->is_shader_model_5_0_support() },
	{ "ANISOTROPIC_SUPPORT", exts->is_anisotropic_filtering_support() },
	{ "FBO_MULTISAMPLE_COVERAGE_SUPPORT", exts->is_fbo_multisample_coverage_support() },
	
	// use sdl platform defines for this
	{ "MAC_OS_X",
#if defined(__MACOSX__) && !defined(A2E_IOS)
		true
#else
		false
#endif
	},
	{ "WINDOWS",
#if defined(__WINDOWS__)
		true
#else
		false
#endif
	},
	{ "LINUX",
#if defined(__LINUX__)
		true
#else
		false
#endif
	},
	{ "IOS",
#if defined(A2E_IOS)
		true
#else
		false
#endif
	},
})
{
}

a2e_shader::~a2e_shader() {
}

a2e_shader::a2e_shader_object* a2e_shader::add_a2e_shader(const string& identifier) {
	a2e_shader_objects.push_back(new a2e_shader::a2e_shader_object());
	a2e_shader_objects.back()->identifier = identifier;
	
	a2e_shaders[identifier].push_back(a2e_shader_objects.back());	
	return a2e_shader_objects.back();
}

a2e_shader::a2e_shader_include_object* a2e_shader::create_a2e_shader_include() {
	a2e_shader_include_objects.push_back(new a2e_shader::a2e_shader_include_object());
	return a2e_shader_include_objects.back();
}

/*! loads and adds a a2eshd file
 *  @param filename the a2eshd file name
 */
bool a2e_shader::load_a2e_shader(const string& identifier, const string& filename, a2e_shader_object* shader_object) {
	// read file data
	stringstream buffer(stringstream::in | stringstream::out | stringstream::binary);
	if(!f->file_to_buffer(filename, buffer)) {
		return false;
	}
	string shader_data(buffer.str());
	
	// replace special chars (&, <, >) and strip xml comments
	static const set<string> valid_tags {
		"?xml", "!DOCTYPE",
		"a2e_shader_include", "a2e_shader", "includes", "options",
		"vertex_shader", "geometry_shader", "fragment_shader",
		"header", "option", "condition"
	};
	
	// TODO: remove this gcc workaround when gcc starts to conform to the spec
#if !defined(__clang__)
	const auto str_insert = [&shader_data](string::iterator& iter, std::initializer_list<char> char_list) {
		iter++;
		for(const auto& elem : char_list) {
			iter = shader_data.insert(iter, elem);
			iter++;
		}
	};
#endif
	
	bool inside_tag = false;
	bool inside_comment = false;
	size_t hyphen_count = 0;
	for(auto iter = begin(shader_data); iter != end(shader_data); ) {
		if(inside_comment && *iter != '>') {
			if(*iter == '-') hyphen_count++;
			else hyphen_count = 0;
			iter = shader_data.erase(iter);
			continue;
		}
		
		switch(*iter) {
			case '&':
#if defined(__clang__)
				iter = shader_data.insert(iter+1, { 'a', 'm', 'p', ';' });
#else
				str_insert(iter, { 'a', 'm', 'p', ';' });
#endif
				continue;
			case '<':
				if(inside_tag) break;
				else {
					//
					const size_t cur_pos(iter - begin(shader_data));
					if(cur_pos+4 < shader_data.size() &&
					   shader_data.substr(cur_pos+1, 3) == "!--") {
						inside_tag = true;
						inside_comment = true;
						iter = shader_data.erase(iter, iter+4);
						continue;
					}
					
					// len must at least be 1 (first must be ignored, since it might be '/'
					const size_t space_pos(shader_data.find_first_of(" \t\r\n>/", cur_pos+2));
					if(space_pos != string::npos) {
						const size_t end_tag(shader_data[cur_pos+1] == '/' ? 1 : 0);
						const size_t tag_len(space_pos - cur_pos - 1 - end_tag);
						const string tag_name(shader_data.substr(cur_pos+1+end_tag, tag_len));
						if(valid_tags.count(tag_name) > 0) {
							inside_tag = true;
							iter += tag_len;
							break;
						}
					}
				}
				*iter = '&';
#if defined(__clang__)
				iter = shader_data.insert(iter+1, { 'l', 't', ';' });
#else
				str_insert(iter, { 'l', 't', ';' });
#endif
				continue;
			case '>':
				if(inside_comment) {
					if(hyphen_count >= 2) {
						iter = shader_data.erase(iter);
						inside_comment = false;
						inside_tag = false;
						hyphen_count = 0;
						continue;
					}
				}
				if(inside_tag) {
					inside_tag = false;
					break;
				}
				*iter = '&';
#if defined(__clang__)
				iter = shader_data.insert(iter+1, { 'g', 't', ';' });
#else
				str_insert(iter, { 'g', 't', ';' });
#endif
				continue;
			default: break;
		}
		iter++;
	}
	
	// process data and check if we have a valid xml file
#if !defined(__WINDOWS__)
	xml::xml_doc shd_doc = x->process_data(shader_data);
#else
	// TODO: fix validation on windows
	xml::xml_doc shd_doc = x->process_data(shader_data, false);
#endif
	if(!shd_doc.valid) {
		a2e_error("invalid a2e-shader file %s!", filename.c_str());
		return false;
	}
	
	// check version
	const size_t a2e_shd_version = shd_doc.get<size_t>("a2e_shader.version", 0);
	if(a2e_shd_version != A2E_SHADER_VERSION) {
		a2e_error("wrong version %u in shader %s - should be %u!",
				  a2e_shd_version, filename.c_str(), A2E_SHADER_VERSION);
		return false;
	}
	
	// 
	a2e_shader_object* a2e_shd = shader_object;
	a2e_shd->identifier = identifier;
	
	// process includes
#if defined(A2E_IOS)
	// always include this in glsl es
	if(node_name == "a2e_shader") a2e_shd->includes.push_back("glsles_compat");
#endif
	const string include_str(shd_doc.get<string>("a2e_shader.includes.content", ""));
	if(include_str.length() > 0) {
		const vector<string> includes(core::tokenize(include_str, ' '));
		for(const auto& include : includes) {
			if(include.length() == 0) continue;
			if(a2e_shader_includes.count(include) == 0) {
				a2e_error("unknown include \"%s\" in shader \"%s\"! - will be ignored!", include, identifier);
			}
			else {
				if(find(cbegin(a2e_shd->includes), cend(a2e_shd->includes), include) == cend(a2e_shd->includes)) {
					a2e_shd->includes.push_back(include);
				}
			}
		}
	}
	
	// process (initial) options
	const string options_str(shd_doc.get<string>("a2e_shader.options.content", ""));
	set<string> options { "#" };
	if(options_str.length() > 0) {
		const vector<string> options_tokens(core::tokenize(options_str, ' '));
		if(options_tokens.size() == 0) {
			a2e_error("options tag found, but empty - defaulting to standard option!");
		}
		else {
			// check if options list contains '#'
			const auto std_option = find(cbegin(options_tokens), cend(options_tokens), "#");
			if(std_option == options_tokens.cend()) {
				// no default option, delete it
				options.erase("#");
			}
			
			for(const auto& opt : options_tokens) {
				options.insert(opt);
			}
		}
	}
	
	// create/modify/extend/combine the option list of the actual shader and its includes:
	// in case both the actual shader and the include shader contain an options list:
	//	* both don't contain a default version: only the intersection will be processed and compiled
	//		example: (A B C) (A C D) -> (AA CC), valid options: (A C)
	//	* only the include contains a default version: the intersection + the include default version
	//		and all remaining actual shader options will be compiled
	//		example: (A B C) (# B C D) -> (A# BB CC), valid options: (A B C)
	//	* only the actual shader contains a default version: this effectively removes the default version,
	//		and all include options will be compiled with the actual shaders default version, excluding
	//		the intersection, which will be compiled together separately
	//		example: (# A B) (B C D E) -> (BB #C #D #E), valid options: (B C D E)
	//	* both contain a default version:
	//		both default version will be combined + the intersection of both + all remaining actual shader
	//		options and the include shader default version + all remaining include shader options and the
	//		actual shader default version
	//		example: (# A B) (# B D) -> (## #D A# BB), valid options: (# A B D)
	//	* include contains a *combiner:
	//		*combiners are basically handled like default options, but both a version w/o them and a version w/ them
	//		will be produced (former: w/o *combiner suffix (as if it didn't exist), latter: w/ *combiner suffix)
	//		furthermore, all prior rules apply and *combiners are applied at the end
	//		example: (# A B) (# *combiner) -> (## A# B# #*combiner A*combiner B*combiner)
	//		example^2: (# A) (*combiner *combiner2) ->
	//		           (# A #*combiner A*combiner #*combiner2 A*combiner2 #*combiner*combiner2 A*combiner*combiner2)
	bool default_option(options.count("#") == 1);
	for(const auto& include : a2e_shd->includes) {
		const a2e_shader_include_object* inc_obj = a2e_shader_includes[include]->shader_include_object;
		const bool inc_default_option(inc_obj->options.count("#") > 0);
		if(!default_option) {
			// shader has no default option any more, create intersection with include
			if(!inc_default_option) {
				// complete intersection
				const set<string> old_options(options);
				for(const auto& opt : old_options) {
					if(inc_obj->options.count(opt) == 0) {
						options.erase(opt);
					}
				}
			}
			else {
				// include has default
				// -> intersection doesn't matter, all shader options are kept
				// -> include options are either already part of shader options or would have been dropped
			}
		}
		else {
			if(!inc_default_option) {
				// drop default
				options.erase("#");
				default_option = false;
				
				// drop all options that aren't part of the include options
				const set<string> old_options(options);
				for(const auto& opt : old_options) {
					if(inc_obj->options.count(opt) == 0) {
						options.erase(opt);
					}
				}
			}
			
			// add missing include options (only those w/o combiners, those will be added later)
			for(const auto& inc_option : inc_obj->options) {
				if(options.count(inc_option) == 0 && inc_option.find("*") == string::npos) {
					options.insert(inc_option);
				}
			}
		}
	}
	
	//
	if(options.empty()) {
		a2e_error("incompatible include options for shader \"%s\" - all options exclude each other!", filename);
		return false;
	}
	
	// handle include combiners (-> merge all combiners)
	for(const auto& include : a2e_shd->includes) {
		const a2e_shader_include_object* inc_obj = a2e_shader_includes[include]->shader_include_object;
		for(const auto& combiner : inc_obj->combiners) {
			if(a2e_shd->combiners.count(combiner) == 0) {
				a2e_shd->combiners.insert(combiner);
			}
		}
	}
	
	// finally: add options to shader options set (-> allocate memory)
	if(!default_option) a2e_shd->remove_option("#");
	for(const auto& opt : options) {
		a2e_shd->add_option(opt);
	}
	
	// find and add all combiners in the shader
	for(const auto& cur_node : shd_doc.nodes) {
		process_node(cur_node.second, nullptr, [&a2e_shd](const xml::xml_node* node) {
			if(node->name() == "option") {
				// only check match options and ignore validity for now
				// -> nomatch combiners would have already been added (if they exist and are active)
				const string match_attrs((*node)["match"]);
				if(match_attrs == "INVALID") return;
				
				const vector<string> combiners(core::tokenize(match_attrs, ' '));
				for(const auto& combiner : combiners) {
					// check if it's a combiner
					if(combiner[0] == '*') {
						a2e_shd->combiners.insert(combiner);
					}
				}
			}
		});
	}
	
	// add option for each combiner combination (combiner power set)
	const set<string> combinations(core::power_set(a2e_shd->combiners));
	set<string> new_options;
	for(const auto& option : a2e_shd->options) {
		for(const auto& comb : combinations) {
			const string combined_option(option+comb);
			new_options.insert(combined_option);
		}
	}
	for(const auto& new_option : new_options) {
		a2e_shd->add_option(new_option);
	}
	
	// create an xml doc from the read data
	xmlDoc* doc = xmlReadMemory(shader_data.c_str(), (unsigned int)shader_data.size(), nullptr,
								(const char*)"UTF-8", 0);
	xmlNode* root = xmlDocGetRootElement(doc);
	
	// parsing time
	for(xmlNode* cur_node = root; cur_node != nullptr; cur_node = cur_node->next) {
		if(cur_node->type == XML_ELEMENT_NODE) {
			xmlElement* cur_elem = (xmlElement*)cur_node;
			string node_name = (const char*)cur_elem->name;
			if(node_name == "a2e_shader") {
				cur_node = cur_node->children;
			}
			else if(node_name == "vertex_shader" ||
					node_name == "geometry_shader" ||
					node_name == "fragment_shader") {
				// get glsl version
#if !defined(A2E_IOS)
				static const ext::GLSL_VERSION default_glsl_version = ext::GLSL_VERSION::GLSL_150;
#else
				static const ext::GLSL_VERSION default_glsl_version = ext::GLSL_VERSION::GLSL_ES_100;
#endif
				ext::GLSL_VERSION glsl_version = default_glsl_version;
				if(x->is_attribute(cur_elem->attributes, "version")) {
					glsl_version = exts->to_glsl_version(x->get_attribute<size_t>(cur_elem->attributes, "version"));
					if(glsl_version == ext::GLSL_VERSION::GLSL_NO_VERSION) {
						// reset to default
						glsl_version = default_glsl_version;
					}
				}
				
				// traverse xml nodes and combine shader code for each option
				for(const auto& option : a2e_shd->options) {
					a2e_shader_code& shd = (node_name == "vertex_shader" ? a2e_shd->vertex_shader[option] :
											(node_name == "geometry_shader" ? a2e_shd->geometry_shader[option] :
											 a2e_shd->fragment_shader[option]));
					shd.version = glsl_version;
					get_shader_content(shd, cur_elem->children, option);
				}
			}
		}
	}
	
	xmlError* err = xmlGetLastError(); // get error before cleanup
	if(err != nullptr) {
		a2e_error("parsing error in a2e-shader file %s!", filename.c_str());
		a2e_error("XML-ERROR: %s\n\rDomain: %d\n\rCode: %d\n\rError-Level: %d\n\rFile: %s (line %d)\n\rAdditional Information: %s %s %s %d %d",
				  err->message, err->domain, err->code, err->level, err->file, err->line, err->str1, err->str2, err->str3, err->int1, err->int2);
		return false;
	}
	
	if(doc != nullptr) xmlFreeDoc(doc);
	xmlCleanupParser();
	
	return true;
}

void a2e_shader::process_node(const xml::xml_node* cur_node, const xml::xml_node* parent a2e_unused, std::function<void(const xml::xml_node* node)> fnc) {
	// process node itself
	fnc(cur_node);
	
	// process child nodes
	for(const auto& child : cur_node->children) {
		process_node(child.second, cur_node, fnc);
	}
}

void a2e_shader::get_shader_content(a2e_shader_code& shd, xmlNode* node, const string& option) {
	deque<xmlNode*> node_stack;
	node_stack.push_back(node);
	
	// split option into <non-combiner><*combiner>...
	const size_t first_comb_pos(option.find("*"));
	const string non_combiner_option(first_comb_pos == string::npos ? option : option.substr(0, first_comb_pos));
	set<string> combiners;
	if(first_comb_pos != string::npos) {
		const vector<string> combiners_vec(core::tokenize(option.substr(first_comb_pos, option.length()-first_comb_pos), '*'));
		for(const auto& combiner : combiners_vec) {
			if(combiner.length() == 0) continue;
			combiners.insert("*"+combiner);
		}
	}
	
	// current code dump string
	string* text = &shd.program;
	
	for(;;) {
		if(node_stack.size() == 0) break;
		
		xmlNode* cur_node = node_stack.back();
		node_stack.pop_back();
		
		while(cur_node != nullptr) {
			const string node_name((const char*)cur_node->name);
			
			if(node_name == "header" ||
			   node_name == "condition" ||
			   node_name == "option") {
				
				bool traverse_child_node = true;
				if(node_name == "header") text = &shd.header;
				else if(node_name == "condition") {
					if(!x->is_attribute(((xmlElement*)cur_node)->attributes, "type") ||
					   !x->is_attribute(((xmlElement*)cur_node)->attributes, "value")) {
						a2e_error("invalid condition (no type and/or value attribute)!");
						traverse_child_node = false;
					}
					else {
						const CONDITION_TYPE condition_type = get_condition_type(x->get_attribute<string>(((xmlElement*)cur_node)->attributes, "type"));
						if(condition_type == CONDITION_TYPE::INVALID) {
							traverse_child_node = false;
						}
						else {
							if(!check_shader_condition(condition_type, x->get_attribute<string>(((xmlElement*)cur_node)->attributes, "value"))) {
								traverse_child_node = false;
							}
						}
					}
				}
				else if(node_name == "option") {
					const bool match_attr(x->is_attribute(((xmlElement*)cur_node)->attributes, "match"));
					const bool nomatch_attr(x->is_attribute(((xmlElement*)cur_node)->attributes, "nomatch"));
					if(!match_attr && !nomatch_attr) {
						a2e_error("option tag found, but no match or nomatch attribute!");
						traverse_child_node = false;
					}
					else if(match_attr && nomatch_attr) {
						a2e_error("option tag found, but both match and nomatch attribute are specified (only one is allowed)!");
						traverse_child_node = false;
					}
					else {
						// check if this is valid for the current option
						const string option_value = x->get_attribute<string>(((xmlElement*)cur_node)->attributes, (match_attr ? "match" : "nomatch"));
						const vector<string> valid_options(core::tokenize(option_value, ' '));
						
						//
						traverse_child_node = !match_attr; // xor to true if found and match, xor to false if found and nomatch
						
						// for non-combiner options:
						if(combiners.empty()) {
							for(const auto& voption : valid_options) {
								if(voption == option) {
									traverse_child_node ^= true;
									break;
								}
							}
						}
						// for combiner options:
						else {
							for(const auto& voption : valid_options) {
								if(voption == option ||
								   voption == non_combiner_option ||
								   combiners.count(voption) > 0) {
									traverse_child_node ^= true;
									break;
								}
							}
						}
					}
				}
				
				if(traverse_child_node) {
					node_stack.push_back(cur_node->next);
					cur_node = cur_node->children;
					continue;
				}
			}
			else if(node_name == "text") {
				*text += (const char*)xmlNodeGetContent(cur_node);
				
				// if we reached the end of a header tag, switch back to program string
				if(cur_node->parent != nullptr && cur_node->next == nullptr &&
				   string((const char*)cur_node->parent->name) == "header") {
					text = &shd.program;
				}
			}
			
			cur_node = cur_node->next;
		}
	}
	
	return;
}

bool a2e_shader::check_shader_condition(const CONDITION_TYPE type, const string& value) const {
	const vector<string> condition_tokens(core::tokenize(value, ' '));
	
	// EQUAL: all strings are matched
	// NEQUAL: no string is matched
	// OR: one (or more) string(s) is/are matched
	// LEQUAL/GEQUAL/NLEQUAL/NGEQUAL: see explanation in shader.h
	switch(type) {
		// EQUAL, NEQUAL and OR apply to all conditions
		case CONDITION_TYPE::EQUAL:
			for(const auto& condition : condition_tokens) {
				if(conditions.count(condition) == 0 ||
				   (conditions.count(condition) == 1 && !conditions.find(condition)->second)) {
					return false;
				}
			}
			return true;
		case CONDITION_TYPE::NEQUAL:
			for(const auto& condition : condition_tokens) {
				if(conditions.count(condition) == 1 && conditions.find(condition)->second) {
					return false;
				}
			}
			return true;
		case CONDITION_TYPE::OR:
			for(const auto& condition : condition_tokens) {
				if(conditions.count(condition) == 1 && conditions.find(condition)->second) {
					return true;
				}
			}
			return false;
			
		// GEQUAL, LEQUAL, NGEQUAL and NLEQUAL only apply to graphic cards
		case CONDITION_TYPE::GEQUAL:
		case CONDITION_TYPE::LEQUAL: {
			ext::GRAPHICS_CARD graphics_card = exts->get_graphics_card();
			ext::GRAPHICS_CARD_VENDOR vendor = exts->get_vendor();
			
			unsigned int min_card = 0, max_card = 0;
			if(graphics_card <= ext::max_generic_card) {
				min_card = (unsigned int)ext::min_generic_card;
				max_card = (unsigned int)ext::max_generic_card;
			}
			else if(vendor == ext::GRAPHICS_CARD_VENDOR::NVIDIA && graphics_card <= ext::max_nvidia_card) {
				min_card = (unsigned int)ext::min_nvidia_card;
				max_card = (unsigned int)ext::max_nvidia_card;
			}
			else if(vendor == ext::GRAPHICS_CARD_VENDOR::ATI && graphics_card <= ext::max_ati_card) {
				min_card = (unsigned int)ext::min_ati_card;
				max_card = (unsigned int)ext::max_ati_card;
			}
			else if(vendor == ext::GRAPHICS_CARD_VENDOR::POWERVR && graphics_card <= ext::max_powervr_card) {
				min_card = (unsigned int)ext::min_powervr_card;
				max_card = (unsigned int)ext::max_powervr_card;
			}
			else if(vendor == ext::GRAPHICS_CARD_VENDOR::INTEL && graphics_card <= ext::max_intel_card) {
				min_card = (unsigned int)ext::min_intel_card;
				max_card = (unsigned int)ext::max_intel_card;
			}
			else {
				a2e_error("unknown card %d!", (unsigned int)graphics_card);
				break;
			}
			
			// some better function binding or lambda expressions would really come in handy here ...
			for(const auto& condition : condition_tokens) {
				for(unsigned int card = min_card; card <= max_card; card++) {
					if(condition == ext::GRAPHICS_CARD_DEFINE_STR[card] &&
					   ((type == CONDITION_TYPE::GEQUAL && graphics_card >= (ext::GRAPHICS_CARD)card) ||
						(type == CONDITION_TYPE::LEQUAL && graphics_card <= (ext::GRAPHICS_CARD)card))) {
						return true;
					}
				}
			}
		}
		break;
		case CONDITION_TYPE::NGEQUAL:
			return (check_shader_condition(CONDITION_TYPE::GEQUAL, value) ^ true);
		case CONDITION_TYPE::NLEQUAL:
			return (check_shader_condition(CONDITION_TYPE::LEQUAL, value) ^ true);
		case CONDITION_TYPE::INVALID: break;
	}
	return false;
}

a2e_shader::CONDITION_TYPE a2e_shader::get_condition_type(const string& condition_type) const {
	if(condition_type == "EQUAL") return a2e_shader::CONDITION_TYPE::EQUAL;
	else if(condition_type == "GEQUAL") return a2e_shader::CONDITION_TYPE::GEQUAL;
	else if(condition_type == "LEQUAL") return a2e_shader::CONDITION_TYPE::LEQUAL;
	else if(condition_type == "NGEQUAL") return a2e_shader::CONDITION_TYPE::NGEQUAL;
	else if(condition_type == "NLEQUAL") return a2e_shader::CONDITION_TYPE::NLEQUAL;
	else if(condition_type == "NEQUAL") return a2e_shader::CONDITION_TYPE::NEQUAL;
	else if(condition_type == "OR") return a2e_shader::CONDITION_TYPE::OR;
	else {
		// invalid type
		a2e_error("unknown condition type \"%s\"!", condition_type);
		return a2e_shader::CONDITION_TYPE::INVALID;
	}
}

bool a2e_shader::process_and_compile_a2e_shader(a2e_shader_object* shd) {
	// do this for each option
	for(const auto& option : shd->options) {
		//
		a2e_shader_code& vertex_shd = shd->vertex_shader[option];
		a2e_shader_code& geometry_shd = shd->geometry_shader[option];
		a2e_shader_code& fragment_shd = shd->fragment_shader[option];
		
		// split option into <non-combiner><*combiner>...
		const size_t first_comb_pos(option.find("*"));
		const string non_combiner_option(first_comb_pos == string::npos ? option : option.substr(0, first_comb_pos));
		set<string> combiners;
		if(first_comb_pos != string::npos) {
			const vector<string> combiners_vec(core::tokenize(option.substr(first_comb_pos, option.length()-first_comb_pos), '*'));
			for(const auto& combiner : combiners_vec) {
				if(combiner.length() == 0) continue;
				combiners.insert("*"+combiner);
			}
		}
		
		// add include code
		// (since we're inserting the include at the beginning, do this in reverse order, so the first include is the first in the code/shader)
		for(auto include = shd->includes.crbegin(); include != shd->includes.crend(); include++) {
			const a2e_shader_include* include_shd = a2e_shader_includes[*include];
			const a2e_shader_include_object* include_obj = include_shd->shader_include_object;
			
			// check for option compatibility
			string include_option = option;
			if(include_obj->options.count(include_option) == 0) {
				if(combiners.empty()) {
					// no combiners, no compatible include option -> include must have default option
					if(include_obj->options.count("#") == 0) {
						a2e_error("include \"%s\" has no compatible option to \"%s\" in shader \"%s\"!",
								  *include, option, shd->identifier);
						return false;
					}
					include_option = "#";
				}
				else {
					// create most compatible option*combiners
					include_option = (include_obj->options.count(non_combiner_option) > 0 ? non_combiner_option : "#");
					if(include_option == "#" && include_obj->options.count("#") == 0) {
						a2e_error("include \"%s\" has no compatible option to \"%s\" in shader \"%s\"!",
								  *include, option, shd->identifier);
						return false;
					}
					
					for(const auto& combiner : combiners) {
						if(include_obj->combiners.count(combiner) > 0) {
							include_option += combiner;
						}
					}
					
					if(include_obj->options.count(include_option) == 0) {
						a2e_error("tried to create compatible include option for \"%s\" in include \"%s\", but failed with non-existing \"%s\" in shader \"%s\"!",
								  option, *include, include_option, shd->identifier);
						return false;
					}
				}
			}
			
			const a2e_shader_code& inc_vs = include_obj->vertex_shader.find(include_option)->second;
			const a2e_shader_code& inc_gs = include_obj->geometry_shader.find(include_option)->second;
			const a2e_shader_code& inc_fs = include_obj->fragment_shader.find(include_option)->second;
			
			// copy include
			vertex_shd.header.insert(0, inc_vs.header);
			vertex_shd.program.insert(0, inc_vs.program);
			
			geometry_shd.header.insert(0, inc_gs.header);
			geometry_shd.program.insert(0, inc_gs.program);
			
			fragment_shd.header.insert(0, inc_fs.header);
			fragment_shd.program.insert(0, inc_fs.program);
		}
	}
	
	// compile
	return compile_a2e_shader(shd);
}

bool a2e_shader::compile_a2e_shader(a2e_shader_object* shd) {
	string tmp;
	tmp.reserve(128);
	bool ret = true;
	
	// do this for each option
	for(const auto& option : shd->options) {
		shd->vs_program[option] = "";
		shd->gs_program[option] = "";
		shd->fs_program[option] = "";
		
		a2e_shader_code& vertex_shd = shd->vertex_shader[option];
		a2e_shader_code& geometry_shd = shd->geometry_shader[option];
		a2e_shader_code& fragment_shd = shd->fragment_shader[option];
		
		// check if the shader object contains a geometry shader
		if(geometry_shd.program.find_first_not_of(" \n\r\t") == string::npos ||
		   geometry_shd.program.find("void main(") == string::npos) {
			shd->geometry_shader_available = false;
			geometry_shd.header = "";
			geometry_shd.program = "";
		}
		else shd->geometry_shader_available = true;
		
		// version
#if !defined(A2E_IOS)
		static const string glsl_version_suffix = " core";
#else
		static const string glsl_version_suffix = "";
#endif
		// TODO: 10.8 + kepler allows for: #version 410 core
		
		shd->vs_program[option] += "#version "+string(exts->glsl_version_str_from_glsl_version(vertex_shd.version))+glsl_version_suffix+"\n";
		shd->fs_program[option] += "#version "+string(exts->glsl_version_str_from_glsl_version(fragment_shd.version))+glsl_version_suffix+"\n";
		if(shd->geometry_shader_available) {
			shd->gs_program[option] += "#version "+string(exts->glsl_version_str_from_glsl_version(geometry_shd.version))+glsl_version_suffix+"\n";
		}
		
		// default precision qualifiers (glsl es only)
#if defined(A2E_IOS)
		static const string def_prec_quals = "precision highp float;\nprecision highp int;\nprecision highp sampler2D;\nprecision highp samplerCube;\n";
		shd->vs_program[option] += def_prec_quals;
		shd->fs_program[option] += def_prec_quals;
#endif
		
		// add pre-defines
		shd->vs_program[option] += string("#define ") + ext::GRAPHICS_CARD_VENDOR_DEFINE_STR[(unsigned int)exts->get_vendor()] + string("\n");
		shd->vs_program[option] += string("#define ") + ext::GRAPHICS_CARD_DEFINE_STR[(unsigned int)exts->get_graphics_card()] + string("\n");
		shd->fs_program[option] += string("#define ") + ext::GRAPHICS_CARD_VENDOR_DEFINE_STR[(unsigned int)exts->get_vendor()] + string("\n");
		shd->fs_program[option] += string("#define ") + ext::GRAPHICS_CARD_DEFINE_STR[(unsigned int)exts->get_graphics_card()] + string("\n");
		if(shd->geometry_shader_available) {
			shd->gs_program[option] += string("#define ") + ext::GRAPHICS_CARD_VENDOR_DEFINE_STR[(unsigned int)exts->get_vendor()] + string("\n");
			shd->gs_program[option] += string("#define ") + ext::GRAPHICS_CARD_DEFINE_STR[(unsigned int)exts->get_graphics_card()] + string("\n");
		}
		
		// header
		shd->vs_program[option] += vertex_shd.header;
		shd->fs_program[option] += fragment_shd.header;
		if(shd->geometry_shader_available) shd->gs_program[option] += geometry_shd.header;
		
		// programs
		shd->vs_program[option] += vertex_shd.program;
		shd->fs_program[option] += fragment_shd.program;
		if(shd->geometry_shader_available) shd->gs_program[option] += geometry_shd.program;
		
#if defined(A2E_IOS)
		make_glsl_es_compat(shd, option);
#endif
		
		// for debugging purposes only ...
#if 0 // TODO: add config setting?
		const string shader_fname = "a2eshader_" + shd->identifier;
		file_io shdfile(string(shader_fname + "_" + option + "_vs.txt").c_str(), file_io::OT_WRITE);
		shdfile.write_block(shd->vs_program[option].c_str(), shd->vs_program[option].size());
		shdfile.close();
#if !defined(A2E_IOS)
		shdfile.open(string(shader_fname + "_" + option + "_gs.txt").c_str(), file_io::OT_WRITE);
		shdfile.write_block(shd->gs_program[option].c_str(), shd->gs_program[option].size());
		shdfile.close();
#endif
		shdfile.open(string(shader_fname + "_" + option + "_fs.txt").c_str(), file_io::OT_WRITE);
		shdfile.write_block(shd->fs_program[option].c_str(), shd->fs_program[option].size());
		shdfile.close();
#endif
		
		ext::GLSL_VERSION max_glsl_version = std::max(std::max(vertex_shd.version, fragment_shd.version), geometry_shd.version);
		shader_object* obj = shader_obj->add_shader_src(shd->identifier, option, max_glsl_version,
														shd->vs_program[option].c_str(),
														shd->gs_program[option].c_str(),
														shd->fs_program[option].c_str());
		if(obj == nullptr) ret = false;
		else obj->a2e_shader = true;
	}
	
	return ret;
}

void a2e_shader::load_a2e_shader_includes() {
	map<string, file_io::FILE_TYPE> file_list = core::get_file_list(e->shader_path("include/"), "a2eshdi");
	for(const auto& include : file_list) {
		const string inc_name = include.first.substr(0, include.first.find(".a2eshdi"));
		a2e_shader_includes[inc_name] = new a2e_shader_include();
		a2e_shader_includes[inc_name]->filename = "include/"+include.first;
	}
	
	for(const auto& include : a2e_shader_includes) {
		include.second->shader_include_object = create_a2e_shader_include();
		load_a2e_shader("a2e_include_"+include.second->filename,
						string(e->shader_path(include.second->filename.c_str())),
						(a2e_shader_object*)include.second->shader_include_object);
	}
}

void a2e_shader::make_glsl_es_compat(a2e_shader_object* shd, const string& option) {
	// this function will try its best to make OpenGL 3.2 / GLSL 1.50 shaders compatible to GLSL ES 1.00
	
	// regex objects
	struct regex_shader_replacement {
		const regex rx;
		const string repl;
	};
	static const regex_shader_replacement vs_regex[] = {
		// strip unavailable storage qualifiers
		{ regex("^([\\s]*)(smooth )([^;]+)"), "$3" }, // NOTE: these precede centroid
		{ regex("^([\\s]*)(flat )([^;]+)"), "$3" },
		{ regex("^([\\s]*)(noperspective )([^;]+)"), "$3" },
		{ regex("^([\\s]*)(centroid )([^;]+)"), "$3" },
		
		{ regex("^([\\s]*)(in )([^;]+)"), "attribute $3" },
		{ regex("^([\\s]*)(out )([^;]+)"), "varying $3" },
	};
	static const regex_shader_replacement fs_regex[] = {
		// strip unavailable storage qualifiers
		{ regex("^([\\s]*)(smooth )([^;]+)"), "$3" },
		{ regex("^([\\s]*)(flat )([^;]+)"), "$3" },
		{ regex("^([\\s]*)(noperspective )([^;]+)"), "$3" },
		{ regex("^([\\s]*)(centroid )([^;]+)"), "$3" },
		
		{ regex("^([\\s]*)(in )([^;]+)"), "varying $3" },
		{ regex("^([\\s]*)(out )([^;]+)"), "// out $3 => gl_FragColor" },
		{ regex("frag_color(_\\d){0,1}"), "gl_FragColor" },
	};
	
	// modify shaders
	for(size_t i = 0; i < 2; i++) {
		string& prog = (i == 0 ? shd->vs_program[option] : shd->fs_program[option]);
		vector<string> lines = core::tokenize(prog, '\n');
		prog = "";
		for(string& line : lines) {
			if(line.size() == 0) {
				prog += '\n';
				continue;
			}
			
			//
			for_each(i == 0 ? begin(vs_regex) : begin(fs_regex),
					 i == 0 ? end(vs_regex) : end(fs_regex),
					 [&line](const regex_shader_replacement& rx) {
						 line = regex_replace(line, rx.rx, rx.repl);
					 });
			prog += line + '\n';
		}
	}
}
