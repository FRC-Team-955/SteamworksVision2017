#include <iostream>
#include <PegFinder.hpp>
#include <Networking.hpp>
#include <string>
#include <pugixml.hpp>
#include <Saving.hpp>
#include <SlidersTwo.hpp>
#include <RealSense.hpp>

int main (int argc, char** argv) {
	//Command args
	if (argc < 2) {
		std::cerr << "Usage: " <<
			"\n\t " << argv[0] << " <Settings.json>" << std::endl;
		return -1;
	} 

	//File saving properties
	std::unordered_map<std::string, int> sliders_save  = {  
		{"hue_slider_lower"	,	179	}, 
		{"hue_slider_upper"	,	179	},
		{"sat_slider_lower"	,	256	},
		{"sat_slider_upper"	,	256	},
		{"val_slider_lower"	,	256	},
		{"val_slider_upper"	,	256	},
		{"area_slider"			,	5000	}
	};

	std::unordered_map<std::string, int> sliders_save_limits  = {  
		{"hue_slider_lower"	,	179	}, 
		{"hue_slider_upper"	,	179	},
		{"sat_slider_lower"	,	256	},
		{"sat_slider_upper"	,	256	},
		{"val_slider_lower"	,	256	},
		{"val_slider_upper"	,	256	},
		{"area_slider"			,	5000	}
	};

	std::unordered_map<std::string, int> sensor_save = { 
		{"depth_width"			,	480	}, 
		{"depth_height"		,	360	}, 
		{"depth_framerate"	,	30		}, 
		{"bgr_width"			,	1920	}, 
		{"bgr_height"			,	1080	}, 
		{"bgr_framerate"		,	30		}, 
		{"exposure"				,	30		} 
	}; 

	std::unordered_map<std::string, int> imgproc_save = {
		{"morph_open"				,	5			},
		{"histogram_min"			,	0			}, 
		{"histogram_max"			,	500		},
		{"histogram_percentile"	,	10			} 
	};

	std::unordered_map<std::string, int> application_options = {
		{"show_sliders"			,	1			}, 
		{"show_rgb"					,	1			}, 
		{"show_depth"				,	1			}, 
		{"show_HSV"					,	1			}, 
		{"show_overlays"			,	1			}, 
	}; 

	std::unordered_map<std::string, std::unordered_map<std::string, int>*> saved_fields = {
		{"Sliders_Limits"				, &sliders_save_limits		},
		{"Sliders"						, &sliders_save				},
		{"Imgproc_const"				, &imgproc_save				},
		{"Application_Options"		, &application_options		},
		{"Sensor"						, &sensor_save	 				}  
	};


	//TODO: Implement a better file saving system! Unordered maps with only ints cannot handle the serial numbers of the cameras
	char serial[11] = "2391000767"; //It's 10 chars long, but there's also the null char

	Realsense* sensor = new Realsense( //TODO: Pass the entire sensor_save object into the class, and use it locally there (Maybe)
			sensor_save["depth_width"		], 
			sensor_save["depth_height"		],
			sensor_save["depth_framerate"	],
			sensor_save["bgr_width"			],
			sensor_save["bgr_height"		], 
			sensor_save["bgr_framerate"	],
			serial
			); 

	Saving* save_file = new Saving(argv[1], &saved_fields);
	if (!save_file->LoadJSON()) {
		save_file->SaveJSON(); 
		std::cerr << "Save file does not exist. Creating defaults..." << std::endl;
	}
	Sliders* interface = new Sliders("Peg_Finder_Sliders", &sliders_save, &sliders_save_limits, save_file); 

	if (application_options["show_sliders"]) {
		interface->InitializeSliders();
	}
	interface->UpdateSliders();

	Networking::Server* serv = new Networking::Server(5806);			

	std::stringstream message_bus;
	PegFinder* finder = new PegFinder(sensor, &message_bus, &saved_fields);
	while(true) {
		std::cout << "Waiting for client connection on port " << 5806 << std::endl;
		serv->WaitForClientConnection();
		while (serv->WaitForClientMessage(&std::cout)) {
			message_bus.str("");
			finder->ProcessFrame();
			serv->SendClientMessage(message_bus.str().c_str());
		}
		std::cout << "Connection stopped. Uh oh." << std::endl;
	}
}
