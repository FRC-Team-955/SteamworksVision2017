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
#include <Median.hpp>

#define PI 3.14159265

using namespace cv;
using namespace MiscImgproc;
using SaveEntry = std::unordered_map<std::string, int>;

class PegFinder {
	private:
		//Runtime objects
		VideoInterface* video_interface; 
		Histogram<unsigned short>* histogram_goal_center;
		Histogram<unsigned short>* hist_inner_roi_left;
		Histogram<unsigned short>* hist_inner_roi_right;
		StripeMatcher* matcher;	
		Median<float>* distance_median;
		Median<float>* angle_median;

		//Create matrices/kernels
		Mat raw_hsv_color; 

		Mat hsv_range_mask; 

		Mat morph_open_struct_element;
		Mat morph_close_struct_element;

		Mat hsv_range_mask_filtered; 
		
		Mat display_buffer; 

		std::vector< std::vector <Point> > contours;

		//TODO: Deallocate all of that dedodated wam (mem leak!)
		std::vector< Rect * > stripes;

		pugi::xml_document* stream_doc; //For serialising to the RIO

		SaveEntry *sliders_save				;	
		SaveEntry *sliders_limits			;	
		SaveEntry *imgproc_save				;	
		SaveEntry *application_options	;	
		SaveEntry *video_interface_save	;	

		float ScoreStripePair (Rect* stripe_A, Rect* stripe_B, float x_bias, float y_bias);

	public:
		PegFinder(VideoInterface* video_interface,
				SaveEntry *sliders_save				,	
				SaveEntry *sliders_limits			,	
				SaveEntry *imgproc_save				,	
				SaveEntry *application_options	,	
				SaveEntry *video_interface_save	,
				pugi::xml_document* stream_doc
				) ;

		void ProcessFrame();

		~PegFinder();
};
#endif
