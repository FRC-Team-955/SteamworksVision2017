#ifndef STRIPEMATCHER_HPP
#define STRIPEMATCHER_HPP
#include <opencv2/opencv.hpp>
#include <vector>

class StripeMatcher {
	private:
		cv::Rect stripe_A_example;
		cv::Rect stripe_B_example;

		float x_bias;
		float y_bias;

		float ScoreStripePair(cv::Rect* stripe_A, cv::Rect* stripe_B);

	public:
		StripeMatcher(float x_bias, float y_bias);

		bool FindPair (std::vector<cv::Rect*>* stripe_list, cv::Rect*& stripe_out_A, cv::Rect*& stripe_out_B);

		~StripeMatcher();
};

#endif
