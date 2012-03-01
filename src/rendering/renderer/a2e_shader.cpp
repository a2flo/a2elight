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
#include <regex>

a2e_shader::a2e_shader(engine* eng) : e(eng), f(e->get_file_io()), exts(e->get_ext()), x(e->get_xml()),
buffer(stringstream::in | stringstream::out | stringstream::binary), buffer2(stringstream::in | stringstream::out | stringstream::binary) {
	
	// add graphic card specific conditions
	conditions[ext::GRAPHIC_CARD_VENDOR_DEFINE_STR[exts->get_vendor()]] = true;
	conditions[ext::GRAPHIC_CARD_DEFINE_STR[exts->get_graphic_card()]] = true;
	conditions["SM50_SUPPORT"] = exts->is_shader_model_5_0_support();
	conditions["ANISOTROPIC_SUPPORT"] = exts->is_anisotropic_filtering_support();
	conditions["FBO_MULTISAMPLE_COVERAGE_SUPPORT"] = exts->is_fbo_multisample_coverage_support();
	
	// use sdl platform defines for this
	conditions["MAC_OS_X"] =
#if defined(__MACOSX__) && !defined(A2E_IOS)
				   true
#else
				   false
#endif
	;
	
	conditions["WINDOWS"] =
#ifdef __WINDOWS__
				   true
#else
				   false
#endif
	;
	
	conditions["LINUX"] =
#ifdef __LINUX__
				   true
#else
				   false
#endif
	;
	
	conditions["IOS"] =
#if defined(A2E_IOS)
	true
#else
	false
#endif
	;
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
	// open file
	if(!f->open_file(filename.c_str(), file_io::OT_READ)) {
		return false;
	}
	
	string shader_data;
	
	core::reset(&buffer);
	f->read_file(&buffer);
	shader_data.reserve((size_t)f->get_filesize());
	shader_data = buffer.str().c_str();
	f->close_file();
	
	// check if we have a valid xml file
	if(shader_data.length() < 5 || shader_data.substr(0, 5) != "<?xml") {
		a2e_error("invalid a2e-shader file %s!", filename.c_str());
		return false;
	}
	
	// replace special chars
	shader_data = core::find_and_replace(shader_data, "&", "&amp;");
	shader_data = core::find_and_replace(shader_data, "< ", "&lt; ");
	shader_data = core::find_and_replace(shader_data, " >", " &gt;");
	shader_data = core::find_and_replace(shader_data, "<= ", "&lt;= ");
	shader_data = core::find_and_replace(shader_data, " >=", " &gt;=");
	
	// 
	a2e_shader_object* a2e_shd = shader_object;
	a2e_shd->identifier = identifier;
	
	// create an xml doc from the read data
	xmlDoc* doc = xmlReadMemory(shader_data.c_str(), (unsigned int)shader_data.size(), nullptr, (const char*)"UTF-8", 0);
	xmlNode* root = xmlDocGetRootElement(doc);
	
	// parsing time
	bool valid_shader = false;
	for(xmlNode* cur_node = root; cur_node; cur_node = cur_node->next) {
		if(cur_node->type == XML_ELEMENT_NODE) {
			xmlElement* cur_elem = (xmlElement*)cur_node;
			string node_name = (const char*)cur_elem->name;
			if(node_name == "a2e_shader" || node_name == "a2e_shader_include") {
				size_t version = x->get_attribute<size_t>(cur_elem->attributes, "version");
				if(version != A2E_SHADER_VERSION) {
					a2e_error("wrong version %u in shader %s - should be %u!",
							 version, filename.c_str(), A2E_SHADER_VERSION);
					if(doc != nullptr) xmlFreeDoc(doc);
					xmlCleanupParser();
					return false;
				}
				
				valid_shader = true;
				cur_node = cur_node->children;
					
#if defined(A2E_IOS)
				// always include this in glsl es
				if(node_name == "a2e_shader") a2e_shd->includes.push_back("glsles_compat");
#endif
			}
			
			if(valid_shader) {
				if(node_name == "includes") {
					string includes_str = (const char*)xmlNodeGetContent((xmlNode*)cur_elem);
					vector<string> includes = core::tokenize(includes_str, ' ');
					for(const auto& include : includes) {
						if(a2e_shader_includes.count(include) == 0) {
							a2e_error("unknown include \"%s\"! - will be ignored!", include);
						}
						else {
							a2e_shd->includes.push_back(include);
						}
					}
				}
				if(node_name == "options") {
					// get shader options list
					if(!x->is_attribute(cur_elem->attributes, "list")) {
						a2e_error("options tag found, but no list attribute! - defaulting to standard option!");
					}
					else {
						const string option_list = x->get_attribute<string>(cur_elem->attributes, "list");
						vector<string> options = core::tokenize(option_list, ' ');
						if(options.size() == 0) {
							a2e_error("options list is empty! - defaulting to standard option!");
						}
						else {
							// check if options list contains '#'
							const auto std_option = find(options.cbegin(), options.cend(), "#");
							if(std_option == options.cend()) {
								// no default option, delete it
								a2e_shd->remove_option("#");
							}
							
							// add options to shader options set, allocate memory
							for(const auto& option : options) {
								a2e_shd->add_option(option);
							}
						}
					}
				}
				else if(node_name == "vertex_shader" ||
						node_name == "geometry_shader" ||
						node_name == "fragment_shader") {
					// get glsl version
#if !defined(A2E_IOS)
					static const ext::GLSL_VERSION default_glsl_version = ext::GLSL_150;
#else
					static const ext::GLSL_VERSION default_glsl_version = ext::GLSL_ES_100;
#endif
					ext::GLSL_VERSION glsl_version = default_glsl_version;
					if(x->is_attribute(cur_elem->attributes, "version")) {
						glsl_version = exts->to_glsl_version(x->get_attribute<size_t>(cur_elem->attributes, "version"));
						if(glsl_version == ext::GLSL_NO_VERSION) {
							// reset to default
							glsl_version = default_glsl_version;
						}
					}
					
					// check for preprocessing
					SHADER_PREPROCESSING shd_preprocessing = a2e_shader::SP_NONE;
					if(x->is_attribute(cur_elem->attributes, "preprocessing")) {
						const string preprocessing = x->get_attribute<string>(cur_elem->attributes, "preprocessing");
						if(preprocessing == "LIGHTING") shd_preprocessing = a2e_shader::SP_LIGHTING;
						else if(preprocessing == "GUI") shd_preprocessing = a2e_shader::SP_GUI;
						else if(preprocessing == "NONE") shd_preprocessing = a2e_shader::SP_NONE;
						else {
							a2e_error("unknown preprocessing method \"%s\"!", preprocessing.c_str());
						}
					}
					
					// traverse xml nodes and combine shader code for each option
					for(const auto& option : a2e_shd->options) {
						a2e_shader_code* shd = (node_name == "vertex_shader" ? a2e_shd->vertex_shader[option] :
												(node_name == "geometry_shader" ? a2e_shd->geometry_shader[option] :
												 a2e_shd->fragment_shader[option]));
						
						shd->version = glsl_version;
						shd->preprocessing = shd_preprocessing;
						
						//
						get_shader_content(shd, cur_elem->children, option);
					}
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
	
	if(!valid_shader) {
		a2e_error("invalid a2e-shader file %s!", filename.c_str());
		return false;
	}
	
	return true;
}

void a2e_shader::get_shader_content(a2e_shader_code* shd, xmlNode* node, const string& option) {
	deque<xmlNode*> node_stack;
	node_stack.push_back(node);
	
	// current code dump string
	string* text = &shd->preprocessor;
	
	for(;;) {
		if(node_stack.size() == 0) break;
		
		xmlNode* cur_node = node_stack.back();
		node_stack.pop_back();
		
		while(cur_node != nullptr) {
			string node_name = (const char*)cur_node->name;
			
			if(node_name == "preprocessor" ||
			   node_name == "variables" ||
			   node_name == "program" ||
			   node_name == "condition" ||
			   node_name == "option") {
				
				bool traverse_child_node = true;
				if(node_name == "preprocessor") text = &shd->preprocessor;
				else if(node_name == "variables") text = &shd->variables;
				else if(node_name == "program") text = &shd->program;
				else if(node_name == "condition") {
					if(!x->is_attribute(((xmlElement*)cur_node)->attributes, "type") ||
					   !x->is_attribute(((xmlElement*)cur_node)->attributes, "value")) {
						a2e_error("invalid condition (no type and/or value attribute)!");
						traverse_child_node = false;
					}
					else {
						A2E_SHADER_CONDITION_TYPE condition_type = get_condition_type(x->get_attribute<string>(((xmlElement*)cur_node)->attributes, "type"));
						if(condition_type == a2e_shader::INVALID) {
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
					if(!x->is_attribute(((xmlElement*)cur_node)->attributes, "value")) {
						a2e_error("option tag found, but no value attribute!");
						traverse_child_node = false;
					}
					else {
						// check if this is valid for the current option
						const string option_value =  x->get_attribute<string>(((xmlElement*)cur_node)->attributes, "value");
						vector<string> valid_options = core::tokenize(option_value, ' ');
						traverse_child_node = false;
						for(const auto& voption : valid_options) {
							if(voption == option) {
								traverse_child_node = true;
								break;
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
			}
			
			cur_node = cur_node->next;
		}
	}
	
	return;
}

bool a2e_shader::check_shader_condition(const A2E_SHADER_CONDITION_TYPE type, const string& value) {
	bool ret = false;
	string tmp;
	string tmp2;
	tmp.reserve(32);
	tmp2.reserve(32);
	
	core::reset(&buffer);
	buffer << value;
	
	// EQUAL: all strings are matched
	// NEQUAL: no string is matched
	// OR: one (or more) string(s) is/are matched
	// LEQUAL/GEQUAL/NLEQUAL/NGEQUAL: see explanation in shader.h
	switch(type) {
		// EQUAL, NEQUAL and OR apply to all conditions
		case a2e_shader::EQUAL:
			ret = true;
			while(buffer >> tmp) {
				if(conditions.count(tmp) == 0 ||
				   (conditions.count(tmp) == 1 && !conditions[tmp])) {
					ret = false;
					break;
				}
			}
			break;
		case a2e_shader::NEQUAL:
			ret = true;
			while(buffer >> tmp) {
				if(conditions.count(tmp) == 1 && conditions[tmp]) {
					ret = false;
					break;
				}
			}
			break;
		case a2e_shader::OR:
			while(buffer >> tmp) {
				if(conditions.count(tmp) == 1 && conditions[tmp]) {
					ret = true;
					break;
				}
			}
			break;
			
		// GEQUAL, LEQUAL, NGEQUAL and NLEQUAL only apply to graphic cards
		case a2e_shader::GEQUAL:
		case a2e_shader::LEQUAL: {
			ext::GRAPHIC_CARD graphic_card = exts->get_graphic_card();
			ext::GRAPHIC_CARD_VENDOR vendor = exts->get_vendor();
			
			ext::GRAPHIC_CARD min_card, max_card;
			if(graphic_card <= ext::max_generic_card) {
				min_card = ext::min_generic_card;
				max_card = ext::max_generic_card;
			}
			else if(vendor == ext::GCV_NVIDIA && graphic_card <= ext::max_nvidia_card) {
				min_card = ext::min_nvidia_card;
				max_card = ext::max_nvidia_card;
			}
			else if(vendor == ext::GCV_ATI && graphic_card <= ext::max_ati_card) {
				min_card = ext::min_ati_card;
				max_card = ext::max_ati_card;
			}
			else if(vendor == ext::GCV_POWERVR && graphic_card <= ext::max_powervr_card) {
				min_card = ext::min_powervr_card;
				max_card = ext::max_powervr_card;
			}
			else {
				a2e_error("unknown card %d!", graphic_card);
				break;
			}
			
			// some better function binding or lambda expressions would really come in handy here ...
			while(buffer >> tmp) {
				for(ssize_t card = min_card; card <= max_card; card++) {
					if(tmp == ext::GRAPHIC_CARD_DEFINE_STR[card] &&
					   ((type == a2e_shader::GEQUAL && graphic_card >= card) ||
						(type == a2e_shader::LEQUAL && graphic_card <= card))) {
						ret = true;
						break;
					}
				}
			}
		}
			break;
		case a2e_shader::NGEQUAL:
			ret = check_shader_condition(a2e_shader::GEQUAL, value) ^ true;
			break;
		case a2e_shader::NLEQUAL:
			ret = check_shader_condition(a2e_shader::LEQUAL, value) ^ true;
			break;
		default:
			break;
	}

	return ret;
}

a2e_shader::A2E_SHADER_CONDITION_TYPE a2e_shader::get_condition_type(const string& condition_type) {
	if(condition_type == "EQUAL") return a2e_shader::EQUAL;
	else if(condition_type == "GEQUAL") return a2e_shader::GEQUAL;
	else if(condition_type == "LEQUAL") return a2e_shader::LEQUAL;
	else if(condition_type == "NGEQUAL") return a2e_shader::NGEQUAL;
	else if(condition_type == "NLEQUAL") return a2e_shader::NLEQUAL;
	else if(condition_type == "NEQUAL") return a2e_shader::NEQUAL;
	else if(condition_type == "OR") return a2e_shader::OR;
	else {
		// invalid type
		a2e_error("unknown condition type \"%s\"!", condition_type.c_str());
		return a2e_shader::INVALID;
	}
}

bool a2e_shader::preprocess_and_compile_a2e_shader(a2e_shader_object* shd) {
	bool ret = true;
	
	// create/modify/extend/combine the option list of the actual shader and its includes:
	// in case of both the actual shader and the include shader contain an options list:
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
	bool default_opt = (shd->options.count("#") != 0);
	for(const auto& include : shd->includes) {
		if(a2e_shader_includes.count(include) == 0) {
			a2e_error("unknown include \"%s\"! - will be ignored!", include);
			continue;
		}
		a2e_shader_include_object* inc_obj = a2e_shader_includes[include]->shader_include_object;
		bool inc_default_opt = (inc_obj->options.count("#") != 0);
		
		// for explanations, see above
		set<string> new_options = shd->options; // we will operate on a copy
		if(!default_opt && !inc_default_opt) {
			// intersect
			for(const auto& option : shd->options) {
				if(inc_obj->options.count(option) == 0) {
					new_options.erase(option);
				}
			}
		}
		else if(!default_opt && inc_default_opt) {
			// intersect + remaining == same/original actual shader set/options
		}
		else if(default_opt && !inc_default_opt) {
			// intersect, remove '#', add remaining include options == include options set
			new_options = inc_obj->options;
		}
		else if(default_opt && inc_default_opt) {
			//
			for(const auto& inc_option : inc_obj->options) {
				if(new_options.count(inc_option) == 0) {
					new_options.insert(inc_option);
				}
			}
		}
		
		// assign/add new options, ...
		set<string> old_options = shd->options; // need to copy again, since we're operating on it
		for(const auto& option : new_options) {
			if(shd->options.count(option) == 0) {
				shd->add_option(option);
				
				// copy data from standard/default option
				*shd->vertex_shader[option] = *shd->vertex_shader["#"];
				*shd->geometry_shader[option] = *shd->geometry_shader["#"];
				*shd->fragment_shader[option] = *shd->fragment_shader["#"];
			}
		}
		// ... delete old ones, ...
		for(const auto& option : old_options) {
			if(new_options.count(option) == 0) {
				shd->remove_option(option);
			}
		}
		// ... and continue with next include
	}
	
	// do this for each option
	for(const auto& option : shd->options) {
		//
		a2e_shader_code* vertex_shd = shd->vertex_shader[option];
		a2e_shader_code* geometry_shd = shd->geometry_shader[option];
		a2e_shader_code* fragment_shd = shd->fragment_shader[option];
		
		// add include code
		// (since we're inserting the include at the beginning, do this in reverse order, so the first include is the first in the code/shader)
		for(auto include = shd->includes.crbegin(); include != shd->includes.crend(); include++) {
			if(a2e_shader_includes.count(*include) == 0) {
				a2e_error("unknown include \"%s\"! - will be ignored!", *include);
				continue;
			}
			
			a2e_shader_include* include_shd = a2e_shader_includes[*include];
			a2e_shader_include_object* include_obj = include_shd->shader_include_object;
			
			// check for option compatibility
			string include_option = option;
			if(include_obj->options.count(option) == 0) {
				if(include_obj->options.count("#") == 0) {
					a2e_error("incompatible include (%s) - no match for option \"%s\" and include contains no standard option!", *include, option);
					continue;
				}
				else include_option = "#";
			}
			
			a2e_shader_code* inc_vs = include_obj->vertex_shader[include_option];
			a2e_shader_code* inc_gs = include_obj->geometry_shader[include_option];
			a2e_shader_code* inc_fs = include_obj->fragment_shader[include_option];
			
			// copy include
			vertex_shd->preprocessor.insert(0, inc_vs->preprocessor);
			vertex_shd->variables.insert(0, inc_vs->variables);
			vertex_shd->program.insert(0, inc_vs->program);
			
			geometry_shd->preprocessor.insert(0, inc_gs->preprocessor);
			geometry_shd->variables.insert(0, inc_gs->variables);
			geometry_shd->program.insert(0, inc_gs->program);
			
			fragment_shd->preprocessor.insert(0, inc_fs->preprocessor);
			fragment_shd->variables.insert(0, inc_fs->variables);
			fragment_shd->program.insert(0, inc_fs->program);
						
			if(vertex_shd->preprocessing == a2e_shader::SP_NONE) vertex_shd->preprocessing = vertex_shd->preprocessing;
			if(geometry_shd->preprocessing == a2e_shader::SP_NONE) geometry_shd->preprocessing = inc_gs->preprocessing;
			if(fragment_shd->preprocessing == a2e_shader::SP_NONE) fragment_shd->preprocessing = inc_fs->preprocessing;
		}
		
		// TODO: check for preprocessing!
		// removed lighting and gui preprocessing, since it isn't needed any more
	}
	
	// compile
	ret = compile_a2e_shader(shd);
	
	return ret;
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
		
		a2e_shader_code* vertex_shd = shd->vertex_shader[option];
		a2e_shader_code* geometry_shd = shd->geometry_shader[option];
		a2e_shader_code* fragment_shd = shd->fragment_shader[option];
		
		// check if the shader object contains a geometry shader
		if(geometry_shd->program.find_first_not_of(" \n\r\t") == string::npos ||
		   geometry_shd->program.find("void main(") == string::npos) {
			shd->geometry_shader_available = false;
			geometry_shd->preprocessor = "";
			geometry_shd->variables = "";
			geometry_shd->program = "";
		}
		else shd->geometry_shader_available = true;
		
		// version
#if !defined(A2E_IOS)
		static const string glsl_version_suffix = " core";
#else
		static const string glsl_version_suffix = "";
#endif
		shd->vs_program[option] += "#version "+string(exts->glsl_version_str_from_glsl_version(vertex_shd->version))+glsl_version_suffix+"\n";
		shd->fs_program[option] += "#version "+string(exts->glsl_version_str_from_glsl_version(fragment_shd->version))+glsl_version_suffix+"\n";
		if(shd->geometry_shader_available) {
			shd->gs_program[option] += "#version "+string(exts->glsl_version_str_from_glsl_version(geometry_shd->version))+glsl_version_suffix+"\n";
		}
		
		// default precision qualifiers (glsl es only)
#if defined(A2E_IOS)
		static const string def_prec_quals = "precision highp float;\nprecision highp int;\nprecision highp sampler2D;\nprecision highp samplerCube;\n";
		shd->vs_program[option] += def_prec_quals;
		shd->fs_program[option] += def_prec_quals;
#endif
		
		// preprocessor
		shd->vs_program[option] += vertex_shd->preprocessor;
		shd->fs_program[option] += fragment_shd->preprocessor;
		if(shd->geometry_shader_available) shd->gs_program[option] += geometry_shd->preprocessor;
		
		// add pre-defines
		shd->vs_program[option] += string("#define ") + ext::GRAPHIC_CARD_VENDOR_DEFINE_STR[exts->get_vendor()] + string("\n");
		shd->vs_program[option] += string("#define ") + ext::GRAPHIC_CARD_DEFINE_STR[exts->get_graphic_card()] + string("\n");
		shd->fs_program[option] += string("#define ") + ext::GRAPHIC_CARD_VENDOR_DEFINE_STR[exts->get_vendor()] + string("\n");
		shd->fs_program[option] += string("#define ") + ext::GRAPHIC_CARD_DEFINE_STR[exts->get_graphic_card()] + string("\n");
		if(shd->geometry_shader_available) {
			shd->gs_program[option] += string("#define ") + ext::GRAPHIC_CARD_VENDOR_DEFINE_STR[exts->get_vendor()] + string("\n");
			shd->gs_program[option] += string("#define ") + ext::GRAPHIC_CARD_DEFINE_STR[exts->get_graphic_card()] + string("\n");
		}
		
		// variables
		shd->vs_program[option] += vertex_shd->variables;
		shd->fs_program[option] += fragment_shd->variables;
		if(shd->geometry_shader_available) shd->gs_program[option] += geometry_shd->variables;
		
		// programs
		shd->vs_program[option] += vertex_shd->program;
		shd->fs_program[option] += fragment_shd->program;
		if(shd->geometry_shader_available) shd->gs_program[option] += geometry_shd->program;
		
		// delete unneeded stuff ...
		shd->vs_program[option] = core::find_and_replace(shd->vs_program[option], "builtin ", "");
		shd->fs_program[option] = core::find_and_replace(shd->fs_program[option], "builtin ", "");
		if(shd->geometry_shader_available) {
			shd->gs_program[option] = core::find_and_replace(shd->gs_program[option], "builtin ", "");
		}
		
#if defined(A2E_IOS)
		make_glsl_es_compat(shd, option);
#endif
		
		// REMOVE ME: for debugging purposes only ...
#if 0
		stringstream shader_fname;
#ifdef __APPLE__
		shader_fname << "a2eshader_" << shd->identifier;
#elif __WINDOWS__
		shader_fname << "A:/albion2/tools/bin/a2eshader_" << cshaders << ".txt";
#else
#endif
		file_io shdfile;
		shdfile.open_file(string(shader_fname.str() + "_vs.txt").c_str(), file_io::OT_WRITE);
		shdfile.write_block(shd->vs_program.begin()->second.c_str(), shd->vs_program.begin()->second.size());
		shdfile.close_file();
		/*f->open_file(string(shader_fname.str() + "_gs.txt").c_str(), file_io::OT_WRITE);
		 f->write_block(shd->gs_program.c_str(), shd->gs_program.size());
		 f->close_file();*/
		shdfile.open_file(string(shader_fname.str() + "_fs.txt").c_str(), file_io::OT_WRITE);
		shdfile.write_block(shd->fs_program.begin()->second.c_str(), shd->fs_program.begin()->second.size());
		shdfile.close_file();
#endif
		
		ext::GLSL_VERSION max_glsl_version = std::max(std::max(vertex_shd->version, fragment_shd->version), geometry_shd->version);
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
	// TODO: !
	
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
