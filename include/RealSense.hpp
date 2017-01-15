#ifndef REALSENSE_HPP
#define REALSENSE_HPP

#include <librealsense/rs.hpp>
#include <opencv2/opencv.hpp>
#include <signal.h>
#include <StringHack.hpp>

using namespace std;
using namespace cv;

class Realsense
{
	private:
		// Realsense stuffs
		rs::context ctx;
		rs::device* dev;
	public:
		// Opencv matrices
		Mat *bgrmatCV;
		Mat *rgbmatCV; 
		Mat *depthmatCV; 
		Mat *registeredCV;
		Mat *largeDepthCV;

		void GrabFrames () ;

		Realsense(int depth_width, int depth_height, int depth_framerate, int bgr_width, int bgr_height, int bgr_framerate);

		// Free up memory/stop processes
		~Realsense();
};

#endif
