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
	dummy_texture = t->add_texture(e->data_path("none.png"), texture_object::TF_LINEAR, e->get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	default_specular = t->add_texture(e->data_path("white.png"), texture_object::TF_LINEAR, e->get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

/*! a2ematerial destructor
 */
a2ematerial::~a2ematerial() {
	for(const auto& material : materials) {
		switch(material.mat_type) {
			case PARALLAX:
				t->delete_texture(((parallax_material*)material.mat)->normal_texture);
				t->delete_texture(((parallax_material*)material.mat)->height_texture);
				// fall through
#if defined(__clang__)
			[[clang::fallthrough]];
#endif
			case DIFFUSE:
				t->delete_texture(((diffuse_material*)material.mat)->diffuse_texture);
				t->delete_texture(((diffuse_material*)material.mat)->specular_texture);
				t->delete_texture(((diffuse_material*)material.mat)->reflectance_texture);
				break;
			default:
				break;
		}

		switch(material.lm_type) {
			case LM_WARD:
				t->delete_texture(((ward_model*)material.model)->isotropic_texture);
				t->delete_texture(((ward_model*)material.model)->anisotropic_texture);
				break;
			case LM_ASHIKHMIN_SHIRLEY:
				t->delete_texture(((ashikhmin_shirley_model*)material.model)->anisotropic_texture);
				break;
			default: break;
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
	file_io f;
	if(!f.file_to_buffer(filename, buffer)) return;
	const string mat_data = buffer.str();
	
	// check if we have a xml (mat) file
	if(mat_data.length() < 5 || mat_data.substr(0, 5) != "<?xml") {
		a2e_error("invalid a2e-material file %s!", filename);
		return;
	}
	
	xmlDoc* doc = xmlReadMemory(mat_data.c_str(), (unsigned int)mat_data.size(), nullptr, (const char*)"UTF-8", 0);
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
				cur_material->id = id;
				cur_material->mat_type = (type == "diffuse" ? DIFFUSE :
										  (type == "parallax" ? PARALLAX :
										   (type == "none" ? NONE : (MATERIAL_TYPE)~0)));
				cur_material->lm_type = (model == "phong" ? LM_PHONG :
										 (model == "ward" ? LM_WARD :
										  (model == "ashikhmin_shirley" ? LM_ASHIKHMIN_SHIRLEY : LM_NONE)));
				
				switch(cur_material->mat_type) {
					case DIFFUSE:
						cur_material->mat = new diffuse_material();
						((diffuse_material*)cur_material->mat)->diffuse_texture = dummy_texture;
						((diffuse_material*)cur_material->mat)->specular_texture = default_specular;
						((diffuse_material*)cur_material->mat)->reflectance_texture = default_specular;
						break;
					case PARALLAX:
						cur_material->mat = new parallax_material();
						((parallax_material*)cur_material->mat)->diffuse_texture = dummy_texture;
						((parallax_material*)cur_material->mat)->specular_texture = default_specular;
						((parallax_material*)cur_material->mat)->reflectance_texture = default_specular;
						((parallax_material*)cur_material->mat)->height_texture = dummy_texture;
						((parallax_material*)cur_material->mat)->normal_texture = dummy_texture;
						break;
					case NONE:
						cur_material->mat = new material_object();
						break;
				}
				
				switch(cur_material->lm_type) {
					case LM_PHONG:
						cur_material->model = new phong_model();
						break;
					case LM_WARD:
						cur_material->model = new ward_model();
						break;
					case LM_ASHIKHMIN_SHIRLEY:
						cur_material->model = new ashikhmin_shirley_model();
						break;
					default: a2e_error("unknown lighting model type %d!", cur_material->lm_type); return;
				}
				
				// get material data
				for(xmlNode* material_node = cur_node->children; material_node; material_node = material_node->next) {
					xmlElement* material_elem = (xmlElement*)material_node;
					string material_name = (const char*)material_elem->name;
					if(material_name == "texture") {
						string texture_filename = x->get_attribute<string>(material_elem->attributes, "file");
						string texture_type_str = x->get_attribute<string>(material_elem->attributes, "type");
						
						TEXTURE_TYPE texture_type = (TEXTURE_TYPE)0;
						if(texture_type_str == "diffuse") texture_type = TT_DIFFUSE;
						else if(texture_type_str == "specular") texture_type = TT_SPECULAR;
						else if(texture_type_str == "reflectance") texture_type = TT_REFLECTANCE;
						else if(texture_type_str == "height") texture_type = TT_HEIGHT;
						else if(texture_type_str == "normal") texture_type = TT_NORMAL;
						else if(texture_type_str == "isotropic") texture_type = TT_ISOTROPIC;
						else if(texture_type_str == "anisotropic") texture_type = TT_ANISOTROPIC;
						else {
							a2e_error("unknown texture type %s!", texture_type_str.c_str());
							return;
						}
						
						// type/mat/model checking
						switch(cur_material->mat_type) {
							case DIFFUSE:
								if(texture_type == TT_HEIGHT ||
								   texture_type == TT_NORMAL) {
									a2e_error("invalid texture type/tag \"%s\" for diffuse material!",
											 texture_type_str.c_str());
									continue;
								}
								break;
							case PARALLAX: // everything allowed for parallax-mapping
							default: break;
						}
						
						switch(cur_material->lm_type) {
							case LM_PHONG:
								if(texture_type == TT_ISOTROPIC ||
								   texture_type == TT_ANISOTROPIC) {
									a2e_error("invalid texture type/tag \"%s\" for phong material (isotropic/anisotropic type is not allowed)!",
											 texture_type_str.c_str());
									continue;
								}
								break;
							case LM_WARD:
								if((texture_type == TT_ISOTROPIC && ((ward_model*)cur_material->model)->type == WT_ANISOTROPIC) ||
								   (texture_type == TT_ANISOTROPIC && ((ward_model*)cur_material->model)->type == WT_ISOTROPIC)) {
									a2e_error("invalid texture type/tag \"%s\" for %s ward material!",
											 texture_type_str.c_str(), (((ward_model*)cur_material->model)->type == WT_ISOTROPIC ? "isotropic" : "anisotropic"));
									continue;
								}
								break;
							case LM_ASHIKHMIN_SHIRLEY:
								if(texture_type == TT_ISOTROPIC) {
									a2e_error("invalid texture type/tag \"%s\" for ashikhmin/shirley material (isotropic type is not allowed)!",
											  texture_type_str.c_str());
									continue;
								}
								break;
							default: break;
						}
						
						// get filtering and wrapping modes (if they are specified)
						GLenum wrap_s = GL_REPEAT, wrap_t = GL_REPEAT; // default values
						texture_object::TEXTURE_FILTERING filtering = texture_object::TF_AUTOMATIC;
						
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
							
							if(filtering_str == "automatic") filtering = texture_object::TF_AUTOMATIC;
							else if(filtering_str == "point") filtering = texture_object::TF_POINT;
							else if(filtering_str == "linear") filtering = texture_object::TF_LINEAR;
							else if(filtering_str == "bilinear") filtering = texture_object::TF_BILINEAR;
							else if(filtering_str == "trilinear") filtering = texture_object::TF_TRILINEAR;
							else {
								a2e_error("unknown filtering mode \"%s\"!", filtering_str.c_str());
							}
						}
						
						a2e_texture tex = t->add_texture(e->data_path(texture_filename.c_str()), filtering, e->get_anisotropic(), wrap_s, wrap_t);
						switch(texture_type) {
							case TT_DIFFUSE: ((diffuse_material*)cur_material->mat)->diffuse_texture = tex; break;
							case TT_SPECULAR: ((diffuse_material*)cur_material->mat)->specular_texture = tex; break;
							case TT_REFLECTANCE: ((diffuse_material*)cur_material->mat)->reflectance_texture = tex; break;
							case TT_HEIGHT: ((parallax_material*)cur_material->mat)->height_texture = tex; break;
							case TT_NORMAL: ((parallax_material*)cur_material->mat)->normal_texture = tex; break;
							case TT_ISOTROPIC: ((ward_model*)cur_material->model)->isotropic_texture = tex; break;
							case TT_ANISOTROPIC: 
								if(cur_material->lm_type == LM_WARD) {
									((ward_model*)cur_material->model)->anisotropic_texture = tex;
								}
								else if(cur_material->lm_type == LM_ASHIKHMIN_SHIRLEY) {
									((ashikhmin_shirley_model*)cur_material->model)->anisotropic_texture = tex;
								}
								break;
						}
					}
					// ward and ashikhmin/shirley only
					else if(material_name == "ward" ||
							material_name == "ashikhmin_shirley" ||
							material_name == "const_isotropic" ||
							material_name == "const_anisotropic") {
						if(cur_material->lm_type != LM_WARD && cur_material->lm_type != LM_ASHIKHMIN_SHIRLEY) {
							a2e_error("<%s> is a ward or ashikhmin/shirley lighting-model only tag!", material_name.c_str());
							continue;
						}
						
						if(material_name == "ward") {
							ward_model* ward = (ward_model*)cur_material->model;
							string ward_type = x->get_attribute<string>(material_elem->attributes, "type");
							if(ward_type == "isotropic") ward->type = WT_ISOTROPIC;
							else if(ward_type == "anisotropic") ward->type = WT_ANISOTROPIC;
							else {
								a2e_error("invalid ward type \"%s\"!", ward_type.c_str());
								return;
							}
						}
						else if(material_name == "ashikhmin_shirley") {
							// no attributes atm
						}
						else if(material_name == "const_isotropic") {
							if(cur_material->lm_type != LM_WARD) {
								a2e_error("invalid lighting model type for \"const_isotropic\" - this is a ward only tag!");
								return;
							}
							ward_model* ward = (ward_model*)cur_material->model;
							ward->isotropic_roughness = string2float(x->get_attribute<string>(material_elem->attributes, "roughness"));
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
							
							if(cur_material->lm_type == LM_WARD) {
								ward_model* ward = (ward_model*)cur_material->model;
								ward->anisotropic_roughness.set(string2float(roughness_arr[0]), string2float(roughness_arr[1]));
							}
							else if(cur_material->lm_type == LM_ASHIKHMIN_SHIRLEY) {
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
						if(cur_material->mat_type != PARALLAX) {
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
		return a2ematerial::NONE;
	}
	return obj->mat->mat_type;
}

a2ematerial::LIGHTING_MODEL a2ematerial::get_lighting_model_type(const size_t& object_id) const {
	const object_mapping* obj = get_object_mapping(object_id);
	if(obj == nullptr) {
		return a2ematerial::LM_NONE;
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
	
	if(obj->mat->mat_type != a2ematerial::PARALLAX) {
		a2e_error("object #%d is not associated to a parallax-mapping material!", object_id);
		return false;
	}
	
	return ((parallax_material*)obj->mat->mat)->parallax_occlusion;
}

void a2ematerial::enable_textures(const size_t& object_id, gl3shader& shd, const size_t texture_mask) const {
	const object_mapping* obj = get_object_mapping(object_id);
	if(obj == nullptr) return;
	
	material* mat = obj->mat;
	switch(mat->mat_type) {
		case DIFFUSE: {
			diffuse_material* dmat = (diffuse_material*)mat->mat;
			if(texture_mask & TT_DIFFUSE) shd->texture("diffuse_texture", dmat->diffuse_texture);
			if(texture_mask & TT_SPECULAR) shd->texture("specular_texture", dmat->specular_texture);
			if(texture_mask & TT_REFLECTANCE) shd->texture("reflectance_texture", dmat->reflectance_texture);
		}
		break;
		case PARALLAX: {
			parallax_material* pmat = (parallax_material*)mat->mat;
			if(texture_mask & TT_DIFFUSE) shd->texture("diffuse_texture", pmat->diffuse_texture);
			if(texture_mask & TT_NORMAL) shd->texture("normal_texture", pmat->normal_texture);
			if(texture_mask & TT_HEIGHT) shd->texture("height_texture", pmat->height_texture);
			if(texture_mask & TT_SPECULAR) shd->texture("specular_texture", pmat->specular_texture);
			if(texture_mask & TT_REFLECTANCE) shd->texture("reflectance_texture", pmat->reflectance_texture);
		}
		break;
		default:
			break;
	}
}

void a2ematerial::disable_textures(const size_t& object_id, const size_t texture_mask) const {
	const object_mapping* obj = get_object_mapping(object_id);
	if(obj == nullptr) return;
	
	// ignore texture_mask for the moment, since this only disables textures
	switch(obj->mat->mat_type) {
		case DIFFUSE:
			glActiveTexture(GL_TEXTURE0);
			glActiveTexture(GL_TEXTURE1);
			break;
		case PARALLAX:
			glActiveTexture(GL_TEXTURE0);
			glActiveTexture(GL_TEXTURE1);
			glActiveTexture(GL_TEXTURE2);
			glActiveTexture(GL_TEXTURE3);
			break;
		default:
			break;
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
