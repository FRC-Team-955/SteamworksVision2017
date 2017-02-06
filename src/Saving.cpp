/*
 * This class was made to make toggling between types of serializers easy
 */

#include "Saving.hpp"

Saving::Saving(std::string directory, std::unordered_map<std::string, SaveEntry *>* load_maps) {
	this->directory = directory;
	this->load_maps = load_maps;
}

void Saving::SaveJSON () {
	std::ofstream save_file (directory);
	save_json.empty();
	for (auto& load_map : *load_maps) {
		for (auto& parameter : *load_map.second) {
			save_json[load_map.first][parameter.first] = parameter.second;
		}
	}
	save_file << save_json.dump(4);
	save_file.close();
}

bool Saving::LoadJSON () {
	save_json.empty();
	std::ifstream load_file (directory);
	if (load_file.good()) {
		std::string json_string ((std::istreambuf_iterator<char>(load_file)), (std::istreambuf_iterator<char>()));
		save_json = json::parse(json_string);
		for (auto& load_map : *load_maps) {
			for (auto& parameter : *load_map.second) {
				if (!save_json[load_map.first][parameter.first].is_null()) {
					(*load_map.second)[parameter.first] = save_json[load_map.first][parameter.first]; 
				}
			}
		}
	} else {
		return false;
	}
	load_file.close();
	return true;
}

void Saving::SaveXML () {
	save_xml.save_file(directory.c_str());
}

void Saving::UpdateXML () {
	save_xml.reset();
	for (auto& load_map : *load_maps) {
		pugi::xml_node child_node = save_xml.append_child(load_map.first.c_str());
		for (auto& parameter : *load_map.second) {
			child_node.append_attribute(parameter.first.c_str()) = load_map.second;
		}
	}
}

bool Saving::LoadXML () {
	//save_xml.reset(); //Just to be sure. The doc says load_file destroys the old tree, but I'm paranoid.
	//pugi::xml_parse_result result = save_xml.load_file(directory.c_str());
	//if (result.status == pugi::xml_parse_status::status_io_error) {
	//	for (auto& load_map : *load_maps) {
	//		pugi::xml_node child_node;
	//		if (child_node = save_xml.child(load_map.first.c_str())) {
	//			for (auto& parameter : *load_map.second) {
	//				if () {
	//					(*load_map.second)[parameter.first] = save_json[load_map.first][parameter.first]; 
	//				}
	//			}
	//		}
	//	}
	//	return true;
	//} else {
	//	return false;
	//}
	return false;
}


//Glorius performance increase
void Saving::StreamXML (std::ostream* out) {
	UpdateXML();
	save_xml.save(*out); 
}

Saving::~Saving () {
	delete load_maps;
}
