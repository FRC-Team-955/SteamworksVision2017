/*
 * This class was made to make transforming between types of serializers easy, by the way
 */

#include "Saving.hpp"

Saving::Saving(std::string directory, std::unordered_map<std::string, std::unordered_map<std::string, int> *>* load_maps) {
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

Saving::~Saving () {
	delete load_maps;
}
