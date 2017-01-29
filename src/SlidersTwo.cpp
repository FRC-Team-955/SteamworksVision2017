#include "SlidersTwo.hpp"

//Stupid workaround
void Sliders::on_trackbar(int newVal, void * object) { 
	Saving* save_object = (Saving*) object;
	save_object->SaveJSON();
};

Sliders::Sliders(const char* window_title, std::unordered_map<std::string, int> *sliders, std::unordered_map<std::string, int> *sliders_limits, Saving* save_object) {
	this->window_title = window_title;
	this->sliders = sliders;
	this->sliders_limits = sliders_limits;
	this->save_object = save_object;
}

void Sliders::InitializeSliders () {
	Mat sliderWidth(1, 1920 / 1, 0); //HACK to make a separate window for the sliders with a custom width
	namedWindow(window_title, CV_WINDOW_AUTOSIZE);
	imshow(window_title, sliderWidth);
	UpdateSliders();
}

void Sliders::UpdateSliders () { //Write over the sliders - opencv's sliders' value inputs are not marked volatile, so we can't change them manually. 
	for (auto& slider_entry : *sliders) {
		cvCreateTrackbar2(slider_entry.first.c_str(), window_title, &slider_entry.second, (*sliders_limits)[slider_entry.first], on_trackbar, save_object);  
	}
}

