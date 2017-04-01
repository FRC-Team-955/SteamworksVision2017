#ifndef PEGFINDER_HPP
#define PEGFINDER_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <unordered_map> //Faster lookup times, O(1) instead of O(log n) !
#include <Histogram.hpp>
#include <math.h>
#include <MiscImgproc.hpp>
#include <StripeMatcher.hpp>
#include <Median.hpp>
#include <Settings.hpp>

#define DEBUG_SHOW_HSV 			FALSE	
#define DEBUG_SHOW_DEPTH 		FALSE
#define DEBUG_SHOW_OVERLAYS	TRUE
#define DEBUG_SHOW_RGB			FALSE
#define DEBUG_SHOW_SLIDERS		FALSE

#define PI 3.141592653589

using namespace cv;
using namespace MiscImgproc;
using SaveEntry = std::unordered_map<std::string, int>;

class PegFinder {
	private:
		//Runtime objects
		Settings* sf;

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

		std::vector< std::vector <Point> > contours;

		std::vector< Rect * > stripes;

	public:
		struct imgproc_results {
			int stripes_found = 0;
			float x_offset_to_target = 0.f;
			float distance_to_target = 0.f;
			float angle_to_target = 0.f;
			float slope_to_target = 0.f;
			float stripe_width = 0.f;
			float target_x_offset = 0.f;
			bool distance_found = 0.f;
		};

		PegFinder(Settings* sf);

		void ProcessFrame(Mat* depth_image, Mat* color_image, Mat* display_buffer, imgproc_results* results);

		~PegFinder();
};
#endif
