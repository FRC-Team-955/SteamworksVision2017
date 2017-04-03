#include <iostream>
#include <PegFinder.hpp>
#include <Networking.hpp>
#include <string>
#include <pugixml.hpp>
#include <RealSense.hpp>
#include <DummyCamera.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <pthread.h>
#include <vector>
#include <sys/stat.h>
#include <SplineCalc.hpp>

#define TEGRA true
Realsense* sensor;

pugi::xml_document send_doc;

Settings* sf;

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
			sf->imgproc_settings_peg_inst.spline_ctrlpt_distance,
			sf->imgproc_settings_peg_inst.spline_time_unit_multiplier,
			sf->imgproc_settings_peg_inst.end_offset
			);

	Mat encoded_buffer (
			sf->sensor_options_peg_inst.bgr_height,
			sf->sensor_options_peg_inst.bgr_width, 
			CV_8UC3);

	VideoWriter color_writer;
	VideoWriter depth_writer;

	std::string filename = "TEST"; //getDateFileName(); //TODO: UNIQUE NAMING THAT IS TEGRA FRIENDLY
	color_writer.open(file_save_dir + filename + "_rgbcap.avi", VideoWriter::fourcc('M','J','P','G'), 30, (*sensor->rgbmatCV).size(), true);
	depth_writer.open(file_save_dir + filename + "_depth.avi", VideoWriter::fourcc('M','J','P','G'), 30, encoded_buffer.size(), true);

	color_writer.write(*sensor->bgrmatCV);
	depth_writer.write(encoded_buffer);

	PegFinder::imgproc_results results;

	Mat display_buffer;

	std::vector<SplineCalc::motion_plan_result> left_tracks;
	std::vector<SplineCalc::motion_plan_result> right_tracks;

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
			//imshow("Color", display_buffer);
			//cv::waitKey(1);
			//results.slope_to_target = -1.0f;
			std::cout << results.distance_to_target << std::endl;
			//results.target_x_offset = 90.0f;
			//results.stripes_found = 2;
			pugi::xml_node root_node = send_doc.append_child("root");	
			root_node.append_attribute("stripes_found") = std::to_string(results.stripes_found).c_str();	
			if (results.stripes_found == 2) {
				left_tracks.clear();
				right_tracks.clear();
				calc->CalcPaths(&left_tracks, &right_tracks, results.slope_to_target, cv::Point2f(results.target_x_offset, results.distance_to_target));

				pugi::xml_node spline_left_node = root_node.append_child("spline_left");	
				pugi::xml_node spline_right_node = root_node.append_child("spline_right");	
				for (int i = 0; i < left_tracks.size(); i++) {
					{
						pugi::xml_node instance = spline_left_node.append_child(("Point" + std::to_string(i)).c_str());
						instance.append_attribute("Distance") = left_tracks[i].compounded_distance;
						instance.append_attribute("Velocity") = left_tracks[i].velocity;
						instance.append_attribute("DeltaT") = left_tracks[i].time_delta;
					}
					{
						pugi::xml_node instance = spline_right_node.append_child(("Point" + std::to_string(i)).c_str());
						instance.append_attribute("Distance") = right_tracks[i].compounded_distance;
						instance.append_attribute("Velocity") = right_tracks[i].velocity;
						instance.append_attribute("DeltaT") = right_tracks[i].time_delta;
					}
				}
			}
			send_doc.save(ss, "", pugi::format_raw);
			//send_doc.save(std::cout);
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
	sf = new Settings();

	SplineCalc* calc = new SplineCalc(
			sf->imgproc_settings_peg_inst.spline_resolution,
			sf->imgproc_settings_peg_inst.spline_wheel_radius,
			sf->imgproc_settings_peg_inst.spline_max_velocity,
			sf->imgproc_settings_peg_inst.spline_wheel_seperation,
			sf->imgproc_settings_peg_inst.spline_ctrlpt_distance,
			sf->imgproc_settings_peg_inst.spline_time_unit_multiplier,
			sf->imgproc_settings_peg_inst.end_offset
			);

	std::vector<SplineCalc::motion_plan_result> left_tracks;
	std::vector<SplineCalc::motion_plan_result> right_tracks;
	calc->CalcPaths(&left_tracks, &right_tracks, -0.5, cv::Point2f(90.0f, 90.0f));
	pugi::xml_node root_node = send_doc.append_child("root");	
	pugi::xml_node spline_left_node = root_node.append_child("spline_left");	
	pugi::xml_node spline_right_node = root_node.append_child("spline_right");	
	for (int i = 0; i < left_tracks.size(); i++) {
		{
			pugi::xml_node instance = spline_left_node.append_child(std::to_string(i).c_str());
			instance.append_attribute("Distance") = left_tracks[i].compounded_distance;
			instance.append_attribute("Velocity") = left_tracks[i].velocity;
			instance.append_attribute("DeltaT") = left_tracks[i].time_delta;
		}
		{
			pugi::xml_node instance = spline_right_node.append_child(std::to_string(i).c_str());
			instance.append_attribute("Distance") = right_tracks[i].compounded_distance;
			instance.append_attribute("Velocity") = right_tracks[i].velocity;
			instance.append_attribute("DeltaT") = right_tracks[i].time_delta;
		}
	}
	send_doc.save(std::cout);
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

	sensor = new Realsense( 
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
			std::string tempmode = serv->WaitForClientMessage();
			//std::cerr << "Client Message: " << tempmode << std::endl;
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


