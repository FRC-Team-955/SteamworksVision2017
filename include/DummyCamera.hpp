#ifndef DUMMYCAMERA_HPP 
#define DUMMYCAMERA_HPP 
#include <VideoInterface.hpp>
#include <opencv2/opencv.hpp>

class DummyCamera : public VideoInterface {
	private:
		const char* directory;
	public:
		DummyCamera(const char* directory, int depth_width, int depth_height, int bgr_width, int bgr_height);

		void GrabFrames();

		~DummyCamera();
};

#endif