//External Libs
#include <librealsense/rs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/core/cuda.hpp>
//#include <opencv_cuda_hack/cudaarithm.hpp>

//Written for this project
#include <RealSense.hpp>
#include <SlidersTwo.hpp>
#include <Histogram.hpp>
#include <Median.hpp>
#include <map>
#include <Saving.hpp> 
#include <vector> 
#include <math.h>
#include <stdlib.h>
#include <unordered_map> //Faster lookup times, O(1) instead of O(log n) !

using namespace cv;

//Runtime objects
Sliders* interface;
Realsense* sensor; Histogram* hist;
Saving* save_file;
Median* median_filter;

bool ToleranceCheck (float input, float expect, float tolerance) {
	return fabs(input-expect) <= tolerance;
}

struct stripe_object {
	Rect ROI;
	stripe_object *paired;
	bool found_other = false;
	bool was_found = false;

	void AssignPair (stripe_object* other) {
		found_other = true;
	}
};

int main (int argc, char** argv) {
	//Command args
	if (argc < 2) {
		std::cerr << "Usage: " <<
			"\n\t " << argv[0] << " <Settings.json>" << std::endl;
		return -1;
	} 
	//File saving properties
	std::unordered_map<string, int> sliders_save  = {  //The default values are also used as the max values for the sliders before the file updates their values, TODO: Change this at some point, it's sloppy
		{"hue_slider_lower"	,	179	}, //"Name", Default (AND Maximum, same number in sliders)
		{"hue_slider_upper"	,	179	},
		{"sat_slider_lower"	,	256	},
		{"sat_slider_upper"	,	256	},
		{"val_slider_lower"	,	256	},
		{"val_slider_upper"	,	256	},
//		{"open_slider"			,	5		},
//		{"close_slider"		,	20		},
//		{"thresh_slider"		,	256	},
//		{"canny_slider" 		,	256	},
		{"area_slider"			,	5000	}
	};

	std::unordered_map<string, int> sensor_save = {
		{"depth_width"			,	480	}, 
		{"depth_height"		,	360	}, 
		{"depth_framerate"	,	30		}, 
		{"bgr_width"			,	1920	}, 
		{"bgr_height"			,	1080	}, 
		{"bgr_framerate"		,	30		} 
	}; 

	std::unordered_map<string, int> imgproc_save = {
		{"morph_open"		,	5			} 
	};

	std::unordered_map<string, int> application_options = {
		{"show_ui"			,	1			} 
	}; 

	std::unordered_map<string, std::unordered_map<string, int>*> saved_fields = {
		{"Sliders"						, &sliders_save				},
		{"Imgproc_const"				, &imgproc_save				},
		{"Application_Options"		, &application_options		},
		{"Sensor"						, &sensor_save	 				}  
	};

	//Object initialization
	save_file = new Saving(argv[1], &saved_fields);

	interface = new Sliders(argv[1], &sliders_save, save_file); //Run this line BEFORE loading from the save file.

	if (!save_file->LoadJSON()) {
		save_file->SaveJSON(); //Create a new file with the defaults
	}

	//TODO: Better damn solution than this
	if (application_options["show_ui"]) {
		interface->InitializeSliders();
	}

	interface->UpdateSliders();

	sensor = new Realsense( //TODO: Pass the entire sensor_save object into the class, and use it locally there (Maybe)
			sensor_save["depth_width"		], 
			sensor_save["depth_height"		],
			sensor_save["depth_framerate"	],
			sensor_save["bgr_width"			],
			sensor_save["bgr_height"		], 
			sensor_save["bgr_framerate"	]
			); 

	//Create matrices/kernels
	Mat raw_hsv_color; 

	Mat hsv_range_mask; 

	Mat morph_open_struct_element = getStructuringElement(MORPH_RECT, Size( 2*imgproc_save["morph_open"] + 1, 2*imgproc_save["morph_open"]+1 ), Point( imgproc_save["morph_open"], imgproc_save["morph_open"] ) ); //Make sure that objects have a certain area

	Mat hsv_range_mask_filtered; 

	vector< vector <Point> > contours;

	while (true) {
		sensor->GrabFrames();
		if (application_options["show_ui"]) {
			imshow("DEPTH", *sensor->largeDepthCV * 6);
		}

		cvtColor (*sensor->rgbmatCV, raw_hsv_color, COLOR_BGR2HSV); // Convert to HSV colorspace
		//TODO: faster sliders_save indexing than using <map>
		//TODO: Implement depth-hsv combination select. See notes/classification for more detail
		inRange (raw_hsv_color,
				Scalar (sliders_save["hue_slider_lower"], sliders_save["sat_slider_lower"], sliders_save["val_slider_lower"]),
				Scalar (sliders_save["hue_slider_upper"], sliders_save["sat_slider_upper"], sliders_save["val_slider_upper"]),
				hsv_range_mask);
		morphologyEx(hsv_range_mask, hsv_range_mask_filtered, MORPH_OPEN, morph_open_struct_element);


		imshow("HSV_RANGE_SELECT", hsv_range_mask_filtered);

		// Find viable contours
		findContours(hsv_range_mask_filtered, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0,0));
		filtered_contours.empty();
		for (auto& contour : contours) {

			Rect bounding_rectangle = boundingRect(contour);		

			Point bounding_rectangle_center = Point(bounding_rectangle.width / 2, bounding_rectangle.height / 2) + bounding_rectangle.tl();

			float rectangle_dim_ratio = (float)bounding_rectangle.width / (float)bounding_rectangle.height;	
			
			//Check if it fits contour criteria (Area, and ROI width to height ratio)
			//	if ( contourArea(contour) >= sliders_save["area_slider"] && ToleranceCheck(rectangle_dim_ratio, 2.0/5.0, 0.1) ) {
			if ( bounding_rectangle.area() >= sliders_save["area_slider"] && ToleranceCheck(rectangle_dim_ratio, 2.0/5.0, 0.1) ) {
				rectangle(*sensor->bgrmatCV, bounding_rectangle, Scalar(0, 255, 128), 2);
				filtered_contours.push_back(contour); //STORE THE AREA TOO
			} else {
				rectangle(*sensor->bgrmatCV, bounding_rectangle, Scalar(0, 0, 255), 2);
			}
		}

		// Pair contours with others (Find closest by most similar Y value, area, and distance to adjacent)

		// Find the most promising pair (Closest, most centered)
		
		imshow("COLOR", *sensor->bgrmatCV);

		waitKey(10);
	}

}
