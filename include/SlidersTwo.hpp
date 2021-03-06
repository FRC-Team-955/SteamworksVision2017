#ifndef SLIDERSTWO_HPP
#define SLIDERSTWO_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <unordered_map>
#include <map>
#include "Saving.hpp"

using namespace cv;
using SaveEntry = std::unordered_map<std::string, int>;

class Sliders {
	private:
		static void on_trackbar(int newVal, void * object);

		const char* window_title;

		SaveEntry *sliders;
		SaveEntry *sliders_limits;
	
		//TODO: Use a better solution, or at least use std::shared_ptr to make it deallocate at the right time :P
		Saving* save_object; //For the on_trackbar save callback

	public:
		std::unordered_map<char*, int> save_file;

		Sliders(const char* window_title, SaveEntry *sliders,SaveEntry *sliders_max, Saving* save_object);

		void InitializeSliders ();

		void UpdateSliders (); //Write over the sliders - opencv's sliders' value inputs are not marked volatile, so we can't change them manually. 
};

#endif // SLIDERSTWO_HPP
