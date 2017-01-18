#include "RealSense.hpp"

void Realsense::GrabFrames () {
	if (ctx.get_device_count() > 0)	{
		dev->wait_for_frames();

		if (!dev->is_streaming()) {
			cout << "[ ERROR ] Streaming stopped" << endl;
		}

		depthmatCV->data = (unsigned char*)dev->get_frame_data(rs::stream::depth);
		rgbmatCV->data = (unsigned char*)dev->get_frame_data(rs::stream::rectified_color);
		//infraredCV->data = (unsigned char*)dev->get_frame_data(rs::stream::infrared2);
		cvtColor(*rgbmatCV, *bgrmatCV, CV_RGB2BGR);
		largeDepthCV->data = (unsigned char*)dev->get_frame_data(rs::stream::depth_aligned_to_color);
	} else {
		cout << "[ ERROR ] No devices connected! " << endl;
	}
}

Realsense::Realsense(int depth_width, int depth_height, int depth_framerate, int bgr_width, int bgr_height, int bgr_framerate) {
	bgrmatCV = new Mat (bgr_height, bgr_width, CV_8UC3);
	rgbmatCV = new Mat (bgr_height, bgr_width, CV_8UC3);
	registeredCV = new Mat (bgr_height, bgr_width, CV_8UC3); 
	largeDepthCV = new Mat (bgr_height, bgr_width, CV_16UC1); 
	depthmatCV = new Mat (depth_height, depth_width, CV_16UC1);
	//infraredCV = new Mat (360, 480, CV_8UC1);

	printf("There are %d connected RealSense devices.\n", ctx.get_device_count());

	if(ctx.get_device_count() == 0)
		exit(-1);

	dev = ctx.get_device(0);
	printf("\nUsing device 0, an %s\n", dev->get_name());
	printf("    Serial number: %s\n", dev->get_serial());
	printf("    Firmware version: %s\n", dev->get_firmware_version());

	//@60FPS, depth at 320x240, color can be 640x480
	//@60FPS, depth at 480x360, color can be 320x240 or 640x480
	//@30FPS, depth at 320x240, color can be 640x480, 1280x720 or 1920x1080
	//@30FPS, depth at 480x360, color can be 320x240, 640x480, 1280x720, or 1920x1080
	dev->enable_stream(rs::stream::depth, depth_width, depth_height, rs::format::z16, depth_framerate);
	dev->enable_stream(rs::stream::color, bgr_width, bgr_height, rs::format::rgb8, bgr_framerate);
	//dev->enable_stream(rs::stream::infrared2, 480, 360, rs::format::y8, bgr_framerate); //Change this line to include a save file option if you do end up using infrared
	
	dev->start();

}

void Realsense::SetDepthExposure(int exposure) {
	dev->set_option(rs::option::r200_lr_exposure, exposure);
}


void Realsense::SetColorExposure(int exposure) {
	dev->set_option(rs::option::color_exposure, exposure);
}

// Free up memory/stop processes
// TODO: Clean up the device object! (dev)
Realsense::~Realsense() {
}
