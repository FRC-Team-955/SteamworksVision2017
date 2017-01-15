//External Libs
#include <librealsense/rs.hpp>
#include <opencv2/opencv.hpp>
//#include <opencv2/core/cuda.hpp>
//#include <opencv_cuda_hack/cudaarithm.hpp>

//Written for this project
#include <RealSense.hpp>
#include <SlidersTwo.hpp>
#include <Histogram.hpp>
#include <Median.hpp>
#include <map>
#include <Saving.hpp> 

using namespace cv;

//Runtime objects
Sliders* interface;
Realsense* sensor; Histogram* hist;
Saving* save_file;
Median* median_filter;

int main (int argc, char** argv) {
	//Command args
	if (argc < 2) {
		std::cerr << "Usage: " <<
			"\n\t " << argv[0] << " <Settings.json>" << std::endl;
		return -1;
	} 
	//File saving properties
	std::map<string, int> sliders_save  = {  //The default values are also used as the max values for the sliders before the file updates their values
		{"hue_slider_lower"	,	179	}, //"Name", Default (AND Maximum, same number)
		{"hue_slider_upper"	,	179	},
		{"sat_slider_lower"	,	256	},
		{"sat_slider_upper"	,	256	},
		{"val_slider_lower"	,	256	},
		{"val_slider_upper"	,	256	},
//		{"open_slider"			,	20		},
//		{"close_slider"		,	20		},
//		{"thresh_slider"		,	256	},
//		{"canny_slider" 		,	256	},
		{"area_slider"			,	9999	}
	};

	std::map<string, int> sensor_save = {
		{"depth_width"			,	480	}, 
		{"depth_height"		,	360	}, 
		{"depth_framerate"	,	30		}, 
		{"bgr_width"			,	1920	}, 
		{"bgr_height"			,	1080	}, 
		{"bgr_framerate"		,	30		} 
	}; 
	std::map<string, std::map<string, int>*> saved_fields = {
		{"Sliders"		, &sliders_save		},
		{"Sensor"		, &sensor_save	 		}  
	};

	//Object initialization
	save_file = new Saving(argv[1], &saved_fields);

	interface = new Sliders(true, argv[1], &sliders_save, save_file); //Run this line BEFORE loading from the save file.

	if (!save_file->LoadJSON()) {
		save_file->SaveJSON(); //Create a new file with the defaults
	}

	interface->UpdateSliders();

	sensor = new Realsense( //TODO: Pass the entire sensor_save object into the class, and use it locally there
			sensor_save["depth_width"		], 
			sensor_save["depth_height"		],
			sensor_save["depth_framerate"	],
			sensor_save["bgr_width"			],
			sensor_save["bgr_height"		], 
			sensor_save["bgr_framerate"	]
			); 

	//Create matrices/kernels
	Mat convolution_kernel_3x3 = Mat::ones (3, 3, CV_32F);
	convolution_kernel_3x3.at<float> (1, 1) = -8.0f;

	Mat color_channels[3];

	Mat *green_channel_raw;

	Mat raw_hsv_color; 

	Mat hsv_range_mask; 

	while (true) {
		//CPU Ops before GPU
		sensor->GrabFrames();
		imshow("COLOR", *sensor->bgrmatCV);
		imshow("DEPTH", *sensor->largeDepthCV * 6);
		cvtColor (*sensor->rgbmatCV, raw_hsv_color, COLOR_BGR2HSV); // Convert to HSV colorspace
	inRange (raw_hsv_color,
				Scalar (sliders_save["hue_slider_lower"], sliders_save["sat_slider_lower"], sliders_save["val_slider_lower"]),
				Scalar (sliders_save["hue_slider_upper"], sliders_save["sat_slider_upper"], sliders_save["val_slider_upper"]),
				hsv_range_mask);

		imshow("HSV_RANGE_SELECT", hsv_range_mask);

		waitKey(10);
	}

}
