#ifndef MISCIMGPROC_HPP
#define MISCIMGPROC_HPP
#include <opencv2/opencv.hpp>
#include <cmath>
namespace MiscImgproc {
	bool ToleranceCheck (float input, float expect, float tolerance);

	float PointDistance (cv::Point* a, cv::Point* b);

	float PointDistance (cv::Point a, cv::Point b);

	float PointDistance (cv::Point2f a, cv::Point2f b);

	float NegativeReciprocal (float slope);

	cv::Point2f MoveAlongLine (bool forward, float distance, float slope, cv::Point2f offset);

	cv::Point MidPoint (cv::Point* a, cv::Point* b);

	cv::Point MidPoint (cv::Point a, cv::Point b);

	cv::Point GetCenter (cv::Rect* rectangle);
}
#endif
