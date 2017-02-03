#include <MiscImgproc.hpp>

bool MiscImgproc::ToleranceCheck (float input, float expect, float tolerance) {
	return fabs(input-expect) <= tolerance;
}

float MiscImgproc::PointDistance (cv::Point* a, cv::Point* b) {
	return cv::norm(*b-*a);
}

float MiscImgproc::PointDistance (cv::Point a, cv::Point b) {
	return cv::norm(b-a);
}

cv::Point MiscImgproc::MidPoint (cv::Point* a, cv::Point* b) {
	return (*a+*b) / 2;
}

cv::Point MiscImgproc::MidPoint (cv::Point a, cv::Point b) {
	return (a+b) / 2;
}

cv::Point MiscImgproc::GetCenter (cv::Rect* rectangle) {
	return (rectangle->br() + rectangle->tl()) / 2;
}
