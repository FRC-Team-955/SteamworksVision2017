#ifndef PEGFINDER_HPP
#define PEGFINDER_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <stdlib.h>
#include <unordered_map> //Faster lookup times, O(1) instead of O(log n) !
#include <Histogram.hpp>
#include <ostream>
#include <VideoInterface.hpp>
#include <pugixml.hpp>
#include <math.h>
#include <MiscImgproc.hpp>
#include <StripeMatcher.hpp>

#define PI 3.14159265

using namespace cv;
using namespace MiscImgproc;

class PegFinder {
	private:
		//Runtime objects
		VideoInterface* video_interface; 
		Histogram* histogram_goal_center;
		Histogram* hist_inner_roi_left;
		Histogram* hist_inner_roi_right;
		StripeMatcher* matcher;	

		//Create matrices/kernels
		Mat raw_hsv_color; 

		Mat hsv_range_mask; 

		Mat morph_open_struct_element;

		Mat hsv_range_mask_filtered; 
		
		Mat display_buffer; 

		std::vector< std::vector <Point> > contours;

		//TODO: Deallocate all of that dedodated wam (mem leak!)
		std::vector< Rect * > stripes;

		pugi::xml_document stream_doc; //For serialising to the RIO

		//TODO: FIND A BETTER WAY TO DO THIS
		std::unordered_map<std::string, int> *sliders_save				;	
		std::unordered_map<std::string, int> *sliders_limits			;	
		std::unordered_map<std::string, int> *imgproc_save				;	
		std::unordered_map<std::string, int> *application_options	;	
		std::unordered_map<std::string, int> *video_interface_save	;	

		float ScoreStripePair (Rect* stripe_A, Rect* stripe_B, float x_bias, float y_bias);

	public:
		PegFinder(VideoInterface* video_interface, std::unordered_map<std::string, std::unordered_map<std::string, int>*>* saved_fields );

		std::string ProcessFrame();

		~PegFinder();
};
#endif
