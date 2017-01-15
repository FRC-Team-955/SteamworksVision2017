#ifndef SAVING_HPP
#define SAVING_HPP
#include "json.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map> //Faster lookup times

using json = nlohmann::json;

class Saving {
	private:
		std::unordered_map<std::string, std::unordered_map<std::string, int> *>* load_maps;
		std::string directory;
		json save_json;
	public:
		Saving(std::string directory, std::unordered_map<std::string, std::unordered_map<std::string, int> *>* load_maps);

		void SaveJSON ();

		bool LoadJSON ();

		~Saving ();
};
#endif
