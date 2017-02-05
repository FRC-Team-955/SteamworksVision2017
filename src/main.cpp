#include <iostream>
#include <PegFinder.hpp>
#include <Networking.hpp>
#include <string>
#include <pugixml.hpp>
#include <Saving.hpp>
#include <SlidersTwo.hpp>
#include <RealSense.hpp>
#include <DummyCamera.hpp>
#include <opencv2/opencv.hpp>


std::unordered_map<std::string, std::unordered_map<std::string, int>*> saved_fields;
std::unordered_map<std::string, int> application_options;
std::unordered_map<std::string, int> imgproc_save;
std::unordered_map<std::string, int> sensor_save;
std::unordered_map<std::string, int> sliders_save_peg_limits;
std::unordered_map<std::string, int> sliders_save_peg;
char* save_file_dir;
Saving* save_file;
Sliders* interface;
VideoInterface* sensor;

void InitializeSaveFile () {
	//File saving fields
	sliders_save_peg  = {  
		{"hue_slider_lower"	,	179	}, 
		{"hue_slider_upper"	,	179	},
		{"sat_slider_lower"	,	256	},
		{"sat_slider_upper"	,	256	},
		{"val_slider_lower"	,	256	},
		{"val_slider_upper"	,	256	},
		{"area_slider"			,	5000	}
	};

	sliders_save_peg_limits  = {  
		{"hue_slider_lower"	,	179	}, 
		{"hue_slider_upper"	,	179	},
		{"sat_slider_lower"	,	256	},
		{"sat_slider_upper"	,	256	},
		{"val_slider_lower"	,	256	},
		{"val_slider_upper"	,	256	},
		{"area_slider"			,	5000	}
	};

	sensor_save = { 
		{"depth_width"			,	480	}, 
		{"depth_height"		,	360	}, 
		{"depth_framerate"	,	30		}, 
		{"bgr_width"			,	1920	}, 
		{"bgr_height"			,	1080	}, 
		{"bgr_framerate"		,	30		}, 
		{"exposure"				,	30		} 
	}; 

	imgproc_save = {
		{"morph_open"				,	5			},
		{"histogram_min"			,	0			}, 
		{"histogram_max"			,	500		},
		{"histogram_percentile"	,	10			} 
	};

	application_options = {
		{"static_test"				,	0	}, 
		{"show_sliders"			,	1	}, 
		{"show_rgb"					,	1	}, 
		{"show_depth"				,	1	}, 
		{"show_HSV"					,	1	}, 
		{"show_overlays"			,	1	}, 
	}; 

	saved_fields = {
		{"Sliders_Limits"				, &sliders_save_peg_limits		},
		{"Sliders_Peg"					, &sliders_save_peg				},
		{"Imgproc_const"				, &imgproc_save				},
		{"Application_Options"		, &application_options		},
		{"Sensor"						, &sensor_save	 				}  
	};

	save_file = new Saving(save_file_dir, &saved_fields);

	if (!save_file->LoadJSON()) {
		std::cerr << "Save file does not exist. Creating defaults..." << std::endl;
		save_file->SaveJSON(); 
		std::cerr << "Finished creating defaults." << std::endl;
	}

}

void ServerMode() {
	char serial[11] = "2391000767"; //It's 10 chars long, but there's also the null char

	sensor = new Realsense( //TODO: Pass the entire sensor_save object into the class, and use it locally there (Maybe)
			sensor_save["depth_width"		], 
			sensor_save["depth_height"		],
			sensor_save["depth_framerate"	],
			sensor_save["bgr_width"			],
			sensor_save["bgr_height"		], 
			sensor_save["bgr_framerate"	],
			serial
			); 

	Networking::Server* serv = new Networking::Server(5806);			

	cv::Mat display_out;
	display_out = *sensor->bgrmatCV;

	PegFinder* finder = new PegFinder(sensor, &saved_fields);
	while(true) {
		std::cout << "Waiting for client connection on port " << 5806 << std::endl;
		serv->WaitForClientConnection();
		while (serv->GetNetState()) {
			std::cout << serv->WaitForClientMessage();
			serv->SendClientMessage(finder->ProcessFrame().c_str());
			cv::waitKey(10);
		}
		std::cout << "Connection stopped. Uh oh." << std::endl;
	}

}

void TestMode(char* rgb_directory, char* depth_directory) {
	sensor = new DummyCamera(
			rgb_directory,
			depth_directory,
			sensor_save["depth_width"		], 
			sensor_save["depth_height"		],
			sensor_save["bgr_width"			],
			sensor_save["bgr_height"		] 
			); 

	PegFinder* finder = new PegFinder(sensor, &saved_fields);

	while(true) {
		sensor->GrabFrames();
		std::cout << finder->ProcessFrame() << std::endl;
		cv::waitKey(10);
	}
}

int main (int argc, char** argv) {
	//Command args
	if (argc < 2) {
		std::cerr << "Usage: " <<
			"\n\t " << argv[0] << " <Settings.json>" << std::endl;
		return -1;
	} 

	save_file_dir = argv[1];

	InitializeSaveFile();

	if (application_options["static_test"] == 1) {
		if (argc < 4) {
			std::cerr << "Test mode active, requires 2 additional arguements." << 
				"\nUsage" << 
				"\n\t" << argv[0] << " <Settings.json> <RGB.png> <Depth.exr>" << std::endl;
			return -1;
		}
	}

	Sliders* interface = new Sliders("Peg_Finder_Sliders", &sliders_save_peg, &sliders_save_peg_limits, save_file); 

	if (application_options["show_sliders"]) {
		interface->InitializeSliders();
	}
	interface->UpdateSliders();

	if (application_options["static_test"] == 1) {
		TestMode(argv[2], argv[3]);
	} else {
		ServerMode();
	}

}


