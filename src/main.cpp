//External Libs
#include <librealsense/rs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//Written for this project
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

using namespace cv;

//Runtime objects
Sliders* interface;
Realsense* sensor; 
Histogram* hist;
Saving* save_file;

static bool ToleranceCheck (float input, float expect, float tolerance) {
	return fabs(input-expect) <= tolerance;
}

//http://answers.opencv.org/question/74400/calculate-the-distance-pixel-between-the-two-edges-lines/ - Turns out it's a better way than maunally doing the pythag. Theorem!
static float PointDistance (Point* a, Point* b) {
	return norm(*b-*a);
}

static Point MidPoint (Point* a, Point* b) {
	return (*a+*b) / 2;
}

struct stripe_object {
	Rect ROI;
	Point center;

	float ScorePair (stripe_object* other) {
		//float position_difference = PointDistance(&center, &other->center);
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
		{"bgr_framerate"		,	30		}, 
		{"exposure"				,	30		} 
	}; 

	std::unordered_map<string, int> imgproc_save = {
		{"morph_open"				,	5			},
		{"histogram_min"			,	0			}, 
		{"histogram_max"			,	500		},
		{"histogram_percentile"	,	10			} 
	};

	std::unordered_map<string, int> application_options = {
		{"show_sliders"			,	1			}, 
		{"show_rgb"					,	1			}, 
		{"show_depth"				,	1			}, 
		{"show_HSV"					,	1			}, 
		{"show_overlays"			,	1			}, 
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
	if (application_options["show_sliders"]) {
		interface->InitializeSliders();
	}

	interface->UpdateSliders();

	hist = new Histogram(imgproc_save["histogram_min"], imgproc_save["histogram_max"]);

	sensor = new Realsense( //TODO: Pass the entire sensor_save object into the class, and use it locally there (Maybe)
			sensor_save["depth_width"		], 
			sensor_save["depth_height"		],
			sensor_save["depth_framerate"	],
			sensor_save["bgr_width"			],
			sensor_save["bgr_height"		], 
			sensor_save["bgr_framerate"	]
			); 
	//sensor->SetColorExposure(sensor_save["exposure"]);

	//Create matrices/kernels
	Mat raw_hsv_color; 

	Mat hsv_range_mask; 

	Mat morph_open_struct_element = getStructuringElement(MORPH_RECT, Size( 2*imgproc_save["morph_open"] + 1, 2*imgproc_save["morph_open"]+1 ), Point( imgproc_save["morph_open"], imgproc_save["morph_open"] ) ); //Make sure that objects have a certain area

	Mat hsv_range_mask_filtered; 

	vector< vector <Point> > contours;

	//TODO: Deallocate all of that dedodated wam (mem leak!)
	vector< stripe_object * > stripes;

	pugi::xml_document stream_doc; //For serialising to the RIO

	while (true) {
		sensor->GrabFrames();
		if (application_options["show_depth"]) {
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


		//TODO: Save file selections of outputs
		if (application_options["show_HSV"]) {
			imshow("HSV_RANGE_SELECT", hsv_range_mask_filtered);
		}

		// Find viable contours
		findContours(hsv_range_mask_filtered, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0,0));
		stripes.clear();
		for (auto& contour : contours) {

			Rect bounding_rectangle = boundingRect(contour);		

			Point bounding_rectangle_center = Point(bounding_rectangle.width / 2, bounding_rectangle.height / 2) + bounding_rectangle.tl();

			float rectangle_dim_ratio = (float)bounding_rectangle.width / (float)bounding_rectangle.height;	

			//Check if it fits contour criteria (Area, and ROI width to height ratio)
			if ( bounding_rectangle.area() >= sliders_save["area_slider"] && ToleranceCheck(rectangle_dim_ratio, 2.0/5.0, 0.3) ) {
				rectangle(*sensor->bgrmatCV, bounding_rectangle, Scalar(0, 255, 255), 2);

				//Create a new stripe object
				stripe_object* stripe = new stripe_object;
				stripe->ROI = bounding_rectangle;
				stripe->center = bounding_rectangle_center;
				stripes.push_back(stripe); 

			} else {
				rectangle(*sensor->bgrmatCV, bounding_rectangle, Scalar(0, 0, 255), 2);
			}
		}

		// Find the best pair (Find closest by most similar Y value, area, and distance to adjacent)
		float best_score = 0.0;
		stripe_object* best_stripe = nullptr;
		stripe_object* best_stripe_pair = nullptr;
		if (stripes.size() > 1) {
			for (auto& stripe : stripes) {
				for (auto& candidate : stripes) {
					if (candidate != stripe) {
						float score = candidate->ScorePair(stripe);
						if (score > best_score) {
							best_stripe = stripe;	
							best_stripe_pair = candidate;	
							best_score = score;
						}
					}
				}
			}
			if (best_stripe && best_stripe_pair) {
				Rect left = best_stripe->ROI;
				Rect right = best_stripe_pair->ROI;
				if (left.x > right.x) {
					std::swap(left, right);
				}
				Rect final_ROI = Rect(left.tl(), right.br());

				if (application_options["show_overlays"]) {
					rectangle(*sensor->bgrmatCV, final_ROI, Scalar(255, 0, 0), 2);  
					line(*sensor->bgrmatCV, best_stripe->center, best_stripe_pair->center, Scalar(0, 0, 255), 3, CV_AA); 
					//putText(*sensor->bgrmatCV, std::to_string(best_score), best_stripe_pair-> center, CV_FONT_HERSHEY_TRIPLEX, 5, Scalar(0, 0, 255)); 
				}

				unsigned short* pixelList = new unsigned short[final_ROI.area()];
				unsigned short *moving_pixelList = pixelList; //A pointer that gets changed, so we copy the start value
				for (int x = final_ROI.tl().x; x < final_ROI.br().x; x++) {
					for (int y = final_ROI.tl().y; y < final_ROI.br().y; y++) {
						*moving_pixelList = sensor->largeDepthCV->at<unsigned short> (y, x);
						moving_pixelList++;
					}
				}
				hist->insert_histogram_data(pixelList, final_ROI.area());

				//TODO: Move this to the save class instead
				stream_doc.reset();
				pugi::xml_node root_node = stream_doc.append_child("Root");
				int distance_to_screen_edge = sensor_save["bgr_width"] / 2;
				int percentile = hist->take_percentile(imgproc_save["histogram_percentile"]);
				root_node.append_attribute("histogram_state") = percentile > 0; 
				root_node.append_attribute("distance") = percentile; 
				root_node.append_attribute("slope") = (float)(best_stripe->center.y - best_stripe_pair->center.y) / (float)(best_stripe->center.x - best_stripe_pair->center.x);
				root_node.append_attribute("x_magnitude") = (float)(MidPoint(&best_stripe->center, &best_stripe_pair->center).x - distance_to_screen_edge) / (float)distance_to_screen_edge; 
				stream_doc.save(std::cout); //TODO: Set to named pipe later
			}
		}

		//Get a box that encapsulates both stripes

		// Find the most promising pair (Closest, most centered)

		//TODO: Make another mat to display on instead of writing over the sensor's mat
		if (application_options["show_rgb"]) {
			imshow("COLOR", *sensor->bgrmatCV);
		}

		waitKey(10);
	}

}
