#include <DummyCamera.hpp>
//TODO: Make this emulate the time stamping
DummyCamera::DummyCamera(const char* bgr_directory, const char* depth_directory, int depth_width, int depth_height, int bgr_width, int bgr_height) {
	this->bgr_directory	 = bgr_directory;
	this->depth_directory = depth_directory;

	bgrmatCV			 = new cv::Mat (cv::imread(bgr_directory, CV_LOAD_IMAGE_COLOR));
	rgbmatCV			 = new cv::Mat (bgr_height, 	bgr_width, 		CV_8UC3);
	cv::cvtColor(*bgrmatCV, *rgbmatCV, CV_BGR2RGB);

	registeredCV	 = new cv::Mat (bgr_height,	bgr_width, 		CV_8UC3); 
	largeDepthCV	 = new cv::Mat (bgr_height, 	bgr_width, 		CV_16UC1); 

	cv::Mat input = cv::imread(depth_directory, CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_ANYDEPTH);
	input.convertTo(*largeDepthCV, CV_16UC1, 1000); //Muliply by 1000 for meters to milimeters 
}	


float DummyCamera::GetTimeStamp() {
	return 0.0;
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
