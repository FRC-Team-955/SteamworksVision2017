#ifndef SAVING_HPP
#define SAVING_HPP
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map> //Faster lookup times
#include <pugixml.hpp> 

using json = nlohmann::json;

class Saving {
	private:
		std::unordered_map<std::string, std::unordered_map<std::string, int> *>* load_maps;
		std::string directory;
		json save_json;
		pugi::xml_document save_xml;
	public:
		Saving(std::string directory, std::unordered_map<std::string, std::unordered_map<std::string, int> *>* load_maps);

		void SaveJSON ();

		bool LoadJSON ();

		void SaveXML ();

		bool LoadXML ();

		void StreamXML (std::ostream* out);

		void UpdateXML ();

		~Saving ();
};
#endif
