#include <DummyCamera.hpp>

DummyCamera::DummyCamera(const char* directory, int depth_width, int depth_height, int bgr_width, int bgr_height) {
	this->directory = directory;
	bgrmatCV			 = new Mat (bgr_height, 	bgr_width, 		CV_8UC3);
	//rgbmatCV			 = new Mat (bgr_height, 	bgr_width, 		CV_8UC3);
	rgbmatCV			 = new Mat (cv::imread(directory, CV_LOAD_IMAGE_COLOR));
	registeredCV	 = new Mat (bgr_height, 	bgr_width, 		CV_8UC3); 
	largeDepthCV	 = new Mat (bgr_height, 	bgr_width, 		CV_16UC1); 
	depthmatCV		 = new Mat (depth_height, 	depth_width, 	CV_16UC1);
}	


void DummyCamera::GrabFrames() {
}

DummyCamera::~DummyCamera() {
	delete[] bgrmatCV			 ;
	delete[] rgbmatCV			 ;
	delete[] registeredCV	 ;
	delete[] largeDepthCV	 ;
	delete[] depthmatCV		 ;
}
