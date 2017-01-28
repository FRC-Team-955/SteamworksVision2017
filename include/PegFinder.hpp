#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <RealSense.hpp>
#include <SlidersTwo.hpp>
#include <map>
#include <Saving.hpp> 
#include <vector> 
#include <math.h>
#include <stdlib.h>
#include <unordered_map> //Faster lookup times, O(1) instead of O(log n) !
#include <Histogram.hpp>
#include <pugixml.hpp>
#include <ostream>

using namespace cv;

class PegFinder {
	private:
		//Runtime objects
		Sliders* interface;
		Realsense* sensor; 
		Histogram* hist_roi;
		Histogram* hist_inner_roi_left;
		Histogram* hist_inner_roi_right;
		Saving* save_file;
		ostream* output_ss;

		//TODO: Move these functions to their own header 
		static bool ToleranceCheck (float input, float expect, float tolerance) {
			return fabs(input-expect) <= tolerance;
		}

		//http://answers.opencv.org/question/74400/calculate-the-distance-pixel-between-the-two-edges-lines/ - Turns out it's a better way than maunally doing the pythag. Theorem!
		static float PointDistance (Point* a, Point* b) {
			return norm(*b-*a);
		}

		static Point MidPoint (Point a, Point b) {
			return (a+b) / 2;
		}

		struct stripe_object {
			Rect ROI;
			Point center;

			float ScorePair (stripe_object* other) {
				//TODO: NO HARDCODED BIAS
				float position_difference_inv = (100.0 / abs(center.y - other->center.y)) + (1.0 / abs(center.x - other->center.x));
				float area_difference_inv = 1.0 / fabs(other->ROI.area() - ROI.area()); 
				if (position_difference_inv > 0) { //If the distance is 0, it's the same object and that's not okay
					return position_difference_inv + area_difference_inv;
					//return (1.0 / area_difference == 0 ? 1.0 : area_difference) * (1.0 / position_difference);
				} else {
					return 0; 
				}
			}

		};

		//Save file handles
		std::unordered_map<string, int> sliders_save; 

		std::unordered_map<string, int> sensor_save;

		std::unordered_map<string, int> imgproc_save;

		std::unordered_map<string, int> application_options;

		std::unordered_map<string, std::unordered_map<string, int>*> saved_fields;

		//Create matrices/kernels
		Mat raw_hsv_color; 

		Mat hsv_range_mask; 

		Mat morph_open_struct_element;

		Mat hsv_range_mask_filtered; 

		vector< vector <Point> > contours;

		//TODO: Deallocate all of that dedodated wam (mem leak!)
		vector< stripe_object * > stripes;

		pugi::xml_document stream_doc; //For serialising to the RIO


	public:
		PegFinder(char* camera_serial, char* save_file_dir, ostream* output_ss);
		
		void ProcessFrame();
};
