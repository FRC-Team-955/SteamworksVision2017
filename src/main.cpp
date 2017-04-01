#include <iostream>
#include <PegFinder.hpp>
#include <Networking.hpp>
#include <string>
#include <pugixml.hpp>
#include <Saving.hpp>
#include <RealSense.hpp>
#include <DummyCamera.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <pthread.h>
#include <vector>
#include <sys/stat.h>
#include <SplineCalc.hpp>

#define TEGRA FALSE
Realsense* sensor;

pugi::xml_document send_doc;

Settings* sf;

//TODO: Move this stuff to it's own class or make it less icky somehow
pthread_mutex_t xml_mutex;
pthread_mutex_t mode_mutex;
pthread_t xml_thread;
PegFinder* finder;
std::string out_string = "";
std::string mode = "Peg\n";

void* finder_thread (void* arg) {
	char* file_save_dir = (char*)arg;
	std::stringstream ss;
	std::string tempmode = "";
	std::string lastmode = "";

	//TODO: Make sure the save file loading is thread safe!
	SplineCalc* calc = new SplineCalc(
			sf->imgproc_settings_peg_inst.spline_resolution,
			sf->imgproc_settings_peg_inst.spline_wheel_radius,
			sf->imgproc_settings_peg_inst.spline_max_velocity,
			sf->imgproc_settings_peg_inst.spline_wheel_seperation,
			sf->imgproc_settings_peg_inst.spline_ctrlpt_distance
			);

	Mat encoded_buffer (
			sf->sensor_options_peg_inst.bgr_height,
			sf->sensor_options_peg_inst.bgr_width, 
			CV_8UC3);

	VideoWriter color_writer;
	VideoWriter depth_writer;
	std::string filename = "TEST"; //getDateFileName();
	color_writer.open(file_save_dir + filename + "_rgbcap.avi", VideoWriter::fourcc('M','J','P','G'), 30, (*sensor->rgbmatCV).size(), true);
	depth_writer.open(file_save_dir + filename + "_depth.avi", VideoWriter::fourcc('M','J','P','G'), 30, encoded_buffer.size(), true);

	color_writer.write(*sensor->bgrmatCV);
	depth_writer.write(encoded_buffer);

	PegFinder::imgproc_results results;

	Mat display_buffer;

	while (true) {
		sensor->GrabFrames();	

		ss.str("");
		ss.clear();
		send_doc.reset();

		pthread_mutex_lock(&mode_mutex);
		tempmode = mode;
		pthread_mutex_unlock(&mode_mutex);

		if (tempmode == "Peg\n") {
			finder->ProcessFrame(sensor->largeDepthCV, sensor->bgrmatCV, &display_buffer, &results);
			std::vector<SplineCalc::motion_plan_result> left_tracks;
			std::vector<SplineCalc::motion_plan_result> right_tracks;
			calc->CalcPaths(&left_tracks, &right_tracks, 5.0f, cv::Point2f(4.5f, 9.0f));

			send_doc.save(ss, "", pugi::format_raw);
			ss << std::endl;

			pthread_mutex_lock(&xml_mutex);
			out_string = ss.str();
			pthread_mutex_unlock(&xml_mutex);

			//cv::waitKey(10);
		} else if (tempmode == "Live\n") {
			//TODO: Use a universal config system to determine the names of OpenCV windows
			imshow("Color", *sensor->bgrmatCV);

			pthread_mutex_lock(&xml_mutex);
			out_string = "Live Mode\n";
			pthread_mutex_unlock(&xml_mutex);

			memcpy(encoded_buffer.data, 
					(*sensor->largeDepthCV).data, 
					sizeof(unsigned short) * encoded_buffer.rows * encoded_buffer.cols);

			color_writer.write(*sensor->bgrmatCV);
			depth_writer.write(encoded_buffer);

			cv::waitKey(1);
		} else {
			pthread_mutex_lock(&mode_mutex);
			mode = lastmode;
			pthread_mutex_unlock(&mode_mutex);

			pthread_mutex_lock(&xml_mutex);
			out_string = "Invalid Mode!\n";
			pthread_mutex_unlock(&xml_mutex);
		}

		//This will not work on the tegra, and it also has issues with camera ambiguity (because of the -d arguement doesn't include a serial number)
#if !TEGRA
		if (lastmode != tempmode) {
			if (tempmode == "Peg") {
				std::string command = "v4l2-ctl --set-ctrl exposure_absolute=" + std::to_string(sf->sensor_options_peg_inst.exposure) + " -d 2";
				system(command.c_str());
			} else if (tempmode == "Live") {
				system("v4l2-ctl --set-ctrl exposure_absolute=130 -d 2");
			}
		}
#endif
		lastmode = tempmode;
	}
	return NULL;
}

int main () {
}

int gmain (int argc, char** argv) {
	//Command args
	if (argc < 2) {
		std::cerr << "Usage: " <<
			"\n\t " << argv[0] << " <Media Save Dir>" << std::endl;
		return -1;
	} 

	sf = new Settings();

	//TODO: DO THIS USING API CALLS EWW
	system("v4l2-ctl --set-ctrl exposure_auto=1 -d 2");
	std::string command = "v4l2-ctl --set-ctrl exposure_absolute=" + std::to_string(sf->sensor_options_peg_inst.exposure) + " -d 2";
	system(command.c_str());

	sensor = new Realsense( //TODO: Pass the entire video_interface_save object into the class, and use it locally there (Maybe)
			sf->sensor_options_peg_inst.depth_width,		 
			sf->sensor_options_peg_inst.depth_height,		
			sf->sensor_options_peg_inst.depth_framerate,	
			sf->sensor_options_peg_inst.bgr_width,			
			sf->sensor_options_peg_inst.bgr_height,		 
			sf->sensor_options_peg_inst.bgr_framerate,	
			sf->sensor_options_peg_inst.serial
			); 

	Networking::Server* serv = new Networking::Server(sf->server_options_inst.server_port);			

	finder = new PegFinder(sf);

	pthread_mutex_init(&xml_mutex, NULL);
	pthread_create(&xml_thread, NULL, &finder_thread, argv[1]);

	while(true) {
		std::cerr << "Waiting for client connection on port " << sf->server_options_inst.server_port << std::endl;
		serv->WaitForClientConnection();
		while (serv->GetNetState()) {
			//std::cerr << "Client Message: " << 
			std::string tempmode = serv->WaitForClientMessage();
			pthread_mutex_lock(&mode_mutex);
			mode = tempmode;
			pthread_mutex_unlock(&mode_mutex);

			pthread_mutex_lock(&xml_mutex);
			serv->SendClientMessage(out_string.c_str());
			//std::cerr << "Server Broadcast: " << out_string << std::endl;
			pthread_mutex_unlock(&xml_mutex);

		}
		std::cerr << "Connection stopped. Uh oh." << std::endl;
	}

}


