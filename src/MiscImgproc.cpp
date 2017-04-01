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

float MiscImgproc::PointDistance (cv::Point2f a, cv::Point2f b) {
	return cv::norm(b-a);
}

float MiscImgproc::NegativeReciprocal(float slope) {
	return -1.0f / slope;
}

cv::Point2f MiscImgproc::MoveAlongLine (bool forward, float distance, float slope, cv::Point2f offset) {
	float k = (forward ? distance : -distance) / sqrtf(1 + pow(slope, 2));
	return offset + (k * cv::Point2f(1, slope));
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
