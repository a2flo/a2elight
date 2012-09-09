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

#include "a2ematerial.h"

/*! a2ematerial constructor
 */
a2ematerial::a2ematerial(engine* eng) : e(eng), t(eng->get_texman()), exts(eng->get_ext()), x(eng->get_xml()) {
	dummy_texture = t->add_texture(e->data_path("none.png"), TEXTURE_FILTERING::LINEAR, e->get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	default_specular = t->add_texture(e->data_path("white.png"), TEXTURE_FILTERING::LINEAR, e->get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

/*! a2ematerial destructor
 */
a2ematerial::~a2ematerial() {
	for(const auto& material : materials) {
		switch(material.mat_type) {
			case MATERIAL_TYPE::PARALLAX:
				t->delete_texture(((parallax_material*)material.mat)->normal_texture);
				t->delete_texture(((parallax_material*)material.mat)->height_texture);
				// fall through
#if defined(__clang__)
			[[clang::fallthrough]];
#endif
			case MATERIAL_TYPE::DIFFUSE:
				t->delete_texture(((diffuse_material*)material.mat)->diffuse_texture);
				t->delete_texture(((diffuse_material*)material.mat)->specular_texture);
				t->delete_texture(((diffuse_material*)material.mat)->reflectance_texture);
				break;
			case MATERIAL_TYPE::NONE: break;
		}

		switch(material.lm_type) {
			case LIGHTING_MODEL::ASHIKHMIN_SHIRLEY:
				t->delete_texture(((ashikhmin_shirley_model*)material.model)->anisotropic_texture);
				break;
			case LIGHTING_MODEL::PHONG:
			case LIGHTING_MODEL::NONE: break;
		}

		if(material.mat != nullptr) delete material.mat;
		if(material.model != nullptr) delete material.model;
	}
	materials.clear();

	for(const auto& m : mapping) {
		delete m.second;
	}
	mapping.clear();
}

/*! loads an .a2mtl material file
 *  @param filename the materials filename
 */
void a2ematerial::load_material(const string& filename_) {
	filename = filename_;
	
	// read mat data
	if(!file_io::file_to_buffer(filename, buffer)) return;
	const string mat_data = buffer.str();
	
	// check if we have a xml (mat) file
	if(mat_data.length() < 5 || mat_data.substr(0, 5) != "<?xml") {
		a2e_error("invalid a2e-material file %s!", filename);
		return;
	}
	
	xmlDoc* doc = xmlReadMemory(mat_data.c_str(), (int)mat_data.size(), nullptr, (const char*)"UTF-8", 0);
	xmlNode* root = xmlDocGetRootElement(doc);
	
	size_t object_count = 0;
	size_t object_id = 0;
	a2ematerial::material* cur_material = nullptr;
	xmlNode* cur_node = nullptr;
	stack<xmlNode*> node_stack;
	node_stack.push(root);
	while(!node_stack.empty()) {
		cur_node = node_stack.top();
		node_stack.pop();

		if(cur_node->next != nullptr) node_stack.push(cur_node->next);

		if(cur_node->type == XML_ELEMENT_NODE) {
			xmlElement* cur_elem = (xmlElement*)cur_node;
			string node_name = (const char*)cur_elem->name;

			if(cur_node->children != nullptr) node_stack.push(cur_node->children);

			if(node_name == "a2e_material") {
				size_t version = x->get_attribute<size_t>(cur_elem->attributes, "version");
				if(version != A2E_MATERIAL_VERSION) {
					a2e_error("wrong version %u in material %s - should be %u!", version, filename, A2E_MATERIAL_VERSION);
					return;
				}
				
				object_count = x->get_attribute<size_t>(cur_elem->attributes, "object_count");
			}
			else if(node_name == "material") {
				// get material info
				size_t id = x->get_attribute<size_t>(cur_elem->attributes, "id");
				string type = x->get_attribute<string>(cur_elem->attributes, "type");
				string model = x->get_attribute<string>(cur_elem->attributes, "model");
				
				// create material
				materials.push_back(*new material());
				cur_material = &materials.back();
				cur_material->id = (ssize_t)id;
				cur_material->mat_type = (type == "diffuse" ? MATERIAL_TYPE::DIFFUSE :
										  (type == "parallax" ? MATERIAL_TYPE::PARALLAX : MATERIAL_TYPE::NONE));
				cur_material->lm_type = (model == "phong" ? LIGHTING_MODEL::PHONG :
										 (model == "ashikhmin_shirley" ? LIGHTING_MODEL::ASHIKHMIN_SHIRLEY : LIGHTING_MODEL::NONE));
				
				switch(cur_material->mat_type) {
					case MATERIAL_TYPE::DIFFUSE:
						cur_material->mat = new diffuse_material();
						((diffuse_material*)cur_material->mat)->diffuse_texture = dummy_texture;
						((diffuse_material*)cur_material->mat)->specular_texture = default_specular;
						((diffuse_material*)cur_material->mat)->reflectance_texture = default_specular;
						break;
					case MATERIAL_TYPE::PARALLAX:
						cur_material->mat = new parallax_material();
						((parallax_material*)cur_material->mat)->diffuse_texture = dummy_texture;
						((parallax_material*)cur_material->mat)->specular_texture = default_specular;
						((parallax_material*)cur_material->mat)->reflectance_texture = default_specular;
						((parallax_material*)cur_material->mat)->height_texture = dummy_texture;
						((parallax_material*)cur_material->mat)->normal_texture = dummy_texture;
						break;
					case MATERIAL_TYPE::NONE:
						cur_material->mat = new material_object();
						break;
				}
				
				switch(cur_material->lm_type) {
					case LIGHTING_MODEL::PHONG:
						cur_material->model = new phong_model();
						break;
					case LIGHTING_MODEL::ASHIKHMIN_SHIRLEY:
						cur_material->model = new ashikhmin_shirley_model();
						break;
					case LIGHTING_MODEL::NONE:
						a2e_error("unknown lighting model type \"%s\" (%d)!",
								  model, cur_material->lm_type);
						return;
				}
				
				// get material data
				for(xmlNode* material_node = cur_node->children; material_node; material_node = material_node->next) {
					xmlElement* material_elem = (xmlElement*)material_node;
					string material_name = (const char*)material_elem->name;
					if(material_name == "texture") {
						string texture_filename = x->get_attribute<string>(material_elem->attributes, "file");
						string texture_type_str = x->get_attribute<string>(material_elem->attributes, "type");
						
						TEXTURE_TYPE texture_type = (TEXTURE_TYPE)0;
						if(texture_type_str == "diffuse") texture_type = TEXTURE_TYPE::DIFFUSE;
						else if(texture_type_str == "specular") texture_type = TEXTURE_TYPE::SPECULAR;
						else if(texture_type_str == "reflectance") texture_type = TEXTURE_TYPE::REFLECTANCE;
						else if(texture_type_str == "height") texture_type = TEXTURE_TYPE::HEIGHT;
						else if(texture_type_str == "normal") texture_type = TEXTURE_TYPE::NORMAL;
						else if(texture_type_str == "anisotropic") texture_type = TEXTURE_TYPE::ANISOTROPIC;
						else {
							a2e_error("unknown texture type %s!", texture_type_str.c_str());
							return;
						}
						
						// type/mat/model checking
						switch(cur_material->mat_type) {
							case MATERIAL_TYPE::DIFFUSE:
								if(texture_type == TEXTURE_TYPE::HEIGHT ||
								   texture_type == TEXTURE_TYPE::NORMAL) {
									a2e_error("invalid texture type/tag \"%s\" for diffuse material!",
											 texture_type_str.c_str());
									continue;
								}
								break;
							case MATERIAL_TYPE::PARALLAX: // everything allowed for parallax-mapping
							case MATERIAL_TYPE::NONE: break;
						}
						
						switch(cur_material->lm_type) {
							case LIGHTING_MODEL::PHONG:
								if(texture_type == TEXTURE_TYPE::ANISOTROPIC) {
									a2e_error("invalid texture type/tag \"%s\" for phong material (anisotropic type is not allowed)!",
											 texture_type_str.c_str());
									continue;
								}
								break;
							case LIGHTING_MODEL::ASHIKHMIN_SHIRLEY:
							case LIGHTING_MODEL::NONE: break;
						}
						
						// get filtering and wrapping modes (if they are specified)
						GLenum wrap_s = GL_REPEAT, wrap_t = GL_REPEAT; // default values
						TEXTURE_FILTERING filtering = TEXTURE_FILTERING::AUTOMATIC;
						
						for(unsigned int i = 0; i < 2; i++) {
							const char* wrap_mode = (i == 0 ? "wrap_s" : "wrap_t");
							GLenum& wrap_ref = (i == 0 ? wrap_s : wrap_t);
							if(x->is_attribute(material_elem->attributes, wrap_mode)) {
								string wrap_str = x->get_attribute<string>(material_elem->attributes, wrap_mode);
								
								if(wrap_str == "clamp_to_edge") wrap_ref = GL_CLAMP_TO_EDGE;
								else if(wrap_str == "repeat") wrap_ref = GL_REPEAT;
								else if(wrap_str == "mirrored_repeat") wrap_ref = GL_MIRRORED_REPEAT;
								else {
									a2e_error("unknown wrap mode \"%s\" for %s!", wrap_str.c_str(), wrap_mode);
								}
							}
						}
						
						if(x->is_attribute(material_elem->attributes, "filtering")) {
							string filtering_str = x->get_attribute<string>(material_elem->attributes, "filtering");
							
							if(filtering_str == "automatic") filtering = TEXTURE_FILTERING::AUTOMATIC;
							else if(filtering_str == "point") filtering = TEXTURE_FILTERING::POINT;
							else if(filtering_str == "linear") filtering = TEXTURE_FILTERING::LINEAR;
							else if(filtering_str == "bilinear") filtering = TEXTURE_FILTERING::BILINEAR;
							else if(filtering_str == "trilinear") filtering = TEXTURE_FILTERING::TRILINEAR;
							else {
								a2e_error("unknown filtering mode \"%s\"!", filtering_str.c_str());
							}
						}
						
						a2e_texture tex = t->add_texture(e->data_path(texture_filename.c_str()), filtering, e->get_anisotropic(), wrap_s, wrap_t);
						switch(texture_type) {
							case TEXTURE_TYPE::DIFFUSE: ((diffuse_material*)cur_material->mat)->diffuse_texture = tex; break;
							case TEXTURE_TYPE::SPECULAR: ((diffuse_material*)cur_material->mat)->specular_texture = tex; break;
							case TEXTURE_TYPE::REFLECTANCE: ((diffuse_material*)cur_material->mat)->reflectance_texture = tex; break;
							case TEXTURE_TYPE::HEIGHT: ((parallax_material*)cur_material->mat)->height_texture = tex; break;
							case TEXTURE_TYPE::NORMAL: ((parallax_material*)cur_material->mat)->normal_texture = tex; break;
							case TEXTURE_TYPE::ANISOTROPIC:
								if(cur_material->lm_type == LIGHTING_MODEL::ASHIKHMIN_SHIRLEY) {
									((ashikhmin_shirley_model*)cur_material->model)->anisotropic_texture = tex;
								}
								break;
						}
					}
					// ashikhmin/shirley only
					else if(material_name == "ashikhmin_shirley" ||
							material_name == "const_isotropic" ||
							material_name == "const_anisotropic") {
						if(cur_material->lm_type != LIGHTING_MODEL::ASHIKHMIN_SHIRLEY) {
							a2e_error("<%s> is an ashikhmin/shirley lighting-model only tag!", material_name.c_str());
							continue;
						}
						
						if(material_name == "ashikhmin_shirley") {
							// no attributes atm
						}
						else if(material_name == "const_anisotropic") {
							float2 roughness(1.0f);
							string roughness_str = x->get_attribute<string>(material_elem->attributes, "roughness");
							vector<string> roughness_arr = core::tokenize(roughness_str, ',');
							
							if(roughness_arr.size() < 2 ||
							   roughness_arr[0].length() == 0 ||
							   roughness_arr[1].length() == 0) {
								a2e_error("invalid anisotropic roughness value \"%s\"!", roughness_str.c_str());
								return;
							}
							
							if(cur_material->lm_type == LIGHTING_MODEL::ASHIKHMIN_SHIRLEY) {
								ashikhmin_shirley_model* as_model = (ashikhmin_shirley_model*)cur_material->model;
								as_model->anisotropic_roughness.set(string2float(roughness_arr[0]), string2float(roughness_arr[1]));
							}
							else {
								a2e_error("invalid lighting model type for \"const_anisotropic\"!");
								return;
							}
						}
					}
					// parallax mapping only
					else if(material_name == "pom") {
						if(cur_material->mat_type != MATERIAL_TYPE::PARALLAX) {
							a2e_error("<pom> is a parallax-mapping only tag!");
							continue;
						}
						
						((parallax_material*)cur_material->mat)->parallax_occlusion = (x->get_attribute<string>(material_elem->attributes, "value") == "true" ? true : false);
					}
				}
			}
			else if(node_name == "object") {
				size_t material_id = x->get_attribute<size_t>(cur_elem->attributes, "material_id");
				const material* mat = nullptr;
				try {
					mat = &get_material(material_id);
				}
				catch(...) {
					a2e_error("invalid object mapping for object #%d!", object_id);
					
					if(object_id == 0) return; // no default possible, abort
					
					// set to default and continue
					mat = mapping[0]->mat;
				}
				
				bool blending = false;
				if(x->is_attribute(cur_elem->attributes, "blending")) {
					blending = x->get_attribute<bool>(cur_elem->attributes, "blending");
				}
				
				mapping[object_id] = new object_mapping((material*)mat, blending);
				
				object_id++;
			}
		}
	}
	
	if(object_id == 0) {
		a2e_error("at least one object mapping is required!");
		return;
	}
	
	if(object_count < mapping.size()) {
		a2e_error("less object mappings specified than required by object count!");
		return;
	}
	
	xmlFreeDoc(doc);
	xmlCleanupParser();
}

const string& a2ematerial::get_filename() const {
	return filename;
}

const a2ematerial::material& a2ematerial::get_material(const size_t& material_id) const {
	for(const auto& mat : materials) {
		if(mat.id >= 0 && (size_t)mat.id == material_id) {
			return mat;
		}
	}
	a2e_error("no material with an id #%d exists!", material_id);
	throw a2e_exception("material doesn't exist!");
}

a2ematerial::material& a2ematerial::get_material(const size_t& material_id) {
	for(auto& mat : materials) {
		if(mat.id >= 0 && (size_t)mat.id == material_id) {
			return mat;
		}
	}
	a2e_error("no material with an id #%d exists!", material_id);
	throw a2e_exception("material doesn't exist!");
}

float4 a2ematerial::get_color(const string& color_str) {
	vector<string> colors = core::tokenize(color_str, ',');
	bool alpha = (colors.size() >= 4 && colors[3].length() != 0);
	
	if(colors.size() < 3 ||
	   colors[0].length() == 0 ||
	   colors[1].length() == 0 ||
	   colors[2].length() == 0) {
		a2e_error("invalid color string \"%s\"!", color_str.c_str());
		return float4(0.0f);
	}
	
	return float4(string2float(colors[0]),
				  string2float(colors[1]),
				  string2float(colors[2]),
				  alpha ? string2float(colors[3]) : 0.0f);
}

const a2ematerial::object_mapping* a2ematerial::get_object_mapping(const size_t& object_id) const {
	if(mapping.count(object_id) == 0) {
		a2e_error("no object with an id #%d exists!", object_id);
		return nullptr;
	}
	return mapping.find(object_id)->second;
}

a2ematerial::MATERIAL_TYPE a2ematerial::get_material_type(const size_t& object_id) const {
	const object_mapping* obj = get_object_mapping(object_id);
	if(obj == nullptr) {
		return a2ematerial::MATERIAL_TYPE::NONE;
	}
	return obj->mat->mat_type;
}

a2ematerial::LIGHTING_MODEL a2ematerial::get_lighting_model_type(const size_t& object_id) const {
	const object_mapping* obj = get_object_mapping(object_id);
	if(obj == nullptr) {
		return a2ematerial::LIGHTING_MODEL::NONE;
	}
	return obj->mat->lm_type;
}

const a2ematerial::lighting_model* a2ematerial::get_lighting_model(const size_t& object_id) const {
	const object_mapping* obj = get_object_mapping(object_id);
	if(obj == nullptr) {
		return nullptr;
	}
	return obj->mat->model;
}

bool a2ematerial::is_blending(const size_t& object_id) const {
	const object_mapping* obj = get_object_mapping(object_id);
	if(obj == nullptr) return false;
	return obj->blending;
}

bool a2ematerial::is_parallax_occlusion(const size_t& object_id) const {
	const object_mapping* obj = get_object_mapping(object_id);
	if(obj == nullptr) return false;
	
	if(obj->mat->mat_type != a2ematerial::MATERIAL_TYPE::PARALLAX) {
		a2e_error("object #%d is not associated to a parallax-mapping material!", object_id);
		return false;
	}
	
	return ((parallax_material*)obj->mat->mat)->parallax_occlusion;
}

void a2ematerial::enable_textures(const size_t& object_id, gl3shader& shd, const TEXTURE_TYPE texture_mask) const {
	const object_mapping* obj = get_object_mapping(object_id);
	if(obj == nullptr) return;
	
	material* mat = obj->mat;
	switch(mat->mat_type) {
		case MATERIAL_TYPE::DIFFUSE: {
			diffuse_material* dmat = (diffuse_material*)mat->mat;
			if((unsigned int)(texture_mask & TEXTURE_TYPE::DIFFUSE) != 0) shd->texture("diffuse_texture", dmat->diffuse_texture);
			if((unsigned int)(texture_mask & TEXTURE_TYPE::SPECULAR) != 0) shd->texture("specular_texture", dmat->specular_texture);
			if((unsigned int)(texture_mask & TEXTURE_TYPE::REFLECTANCE) != 0) shd->texture("reflectance_texture", dmat->reflectance_texture);
		}
		break;
		case MATERIAL_TYPE::PARALLAX: {
			parallax_material* pmat = (parallax_material*)mat->mat;
			if((unsigned int)(texture_mask & TEXTURE_TYPE::DIFFUSE) != 0) shd->texture("diffuse_texture", pmat->diffuse_texture);
			if((unsigned int)(texture_mask & TEXTURE_TYPE::NORMAL) != 0) shd->texture("normal_texture", pmat->normal_texture);
			if((unsigned int)(texture_mask & TEXTURE_TYPE::HEIGHT) != 0) shd->texture("height_texture", pmat->height_texture);
			if((unsigned int)(texture_mask & TEXTURE_TYPE::SPECULAR) != 0) shd->texture("specular_texture", pmat->specular_texture);
			if((unsigned int)(texture_mask & TEXTURE_TYPE::REFLECTANCE) != 0) shd->texture("reflectance_texture", pmat->reflectance_texture);
		}
		break;
		case MATERIAL_TYPE::NONE: break;
	}
}

void a2ematerial::disable_textures(const size_t& object_id) const {
	const object_mapping* obj = get_object_mapping(object_id);
	if(obj == nullptr) return;
	
	// ignore texture_mask for the moment, since this only disables textures
	switch(obj->mat->mat_type) {
		case MATERIAL_TYPE::DIFFUSE:
			glActiveTexture(GL_TEXTURE0);
			glActiveTexture(GL_TEXTURE1);
			break;
		case MATERIAL_TYPE::PARALLAX:
			glActiveTexture(GL_TEXTURE0);
			glActiveTexture(GL_TEXTURE1);
			glActiveTexture(GL_TEXTURE2);
			glActiveTexture(GL_TEXTURE3);
			break;
		case MATERIAL_TYPE::NONE: break;
	}
}

void a2ematerial::copy_object_mapping(const size_t& from_object, const size_t& to_object) {
	if(mapping.count(to_object) > 0) {
		delete mapping[to_object];
	}
	const object_mapping* from_mapping = mapping[from_object];
	mapping[to_object] = new object_mapping(from_mapping->mat, from_mapping->blending);
}

void a2ematerial::copy_object_mapping(const size_t& from_object, const vector<size_t>& to_objects) {
	for(const auto& obj : to_objects) {
		copy_object_mapping(from_object, obj);
	}
}

size_t a2ematerial::get_material_count() const {
	return materials.size();
}
