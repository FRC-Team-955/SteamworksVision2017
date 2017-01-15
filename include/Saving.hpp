#ifndef SAVING_HPP
#define SAVING_HPP
#include "json.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

using namespace std;

class Saving {
	private:
		map<string, map<string, int> *>* load_maps;
		string directory;
		json save_json;
	public:
		Saving (string directory, map<string, map<string, int> *>* load_maps);

		void SaveJSON ();

		bool LoadJSON ();

		~Saving ();
};
#endif
