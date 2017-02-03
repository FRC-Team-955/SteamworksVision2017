#ifndef MISCIMGPROC_HPP
#define MISCIMGPROC_HPP
#include <opencv2/opencv.hpp>

namespace MiscImgproc {
	 bool ToleranceCheck (float input, float expect, float tolerance);

	 float PointDistance (cv::Point* a, cv::Point* b);

	 float PointDistance (cv::Point a, cv::Point b);

	 cv::Point MidPoint (cv::Point* a, cv::Point* b);

	 cv::Point MidPoint (cv::Point a, cv::Point b);

	 cv::Point GetCenter (cv::Rect* rectangle);
}

#endif
