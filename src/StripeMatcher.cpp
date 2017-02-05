#include <StripeMatcher.hpp>

StripeMatcher::StripeMatcher(float x_bias, float y_bias) {
	this->stripe_A_example = stripe_A_example;
	this->stripe_B_example = stripe_B_example;
	this->x_bias = x_bias;
	this->y_bias = y_bias;
}

float StripeMatcher::ScoreStripePair(cv::Rect* stripe_A, cv::Rect* stripe_B) {
	cv::Point stripe_A_center = (stripe_A->tl() + stripe_A->br()) / 2;
	cv::Point stripe_B_center = (stripe_B->tl() + stripe_B->br()) / 2;

	float position_difference_inv =  //Biased position difference score (Bigger is worse)
		(y_bias / abs(stripe_A_center.y - stripe_B_center.y)) + //Difference between the stripe centers, biased by Y (Larger constant means more bias)
		(x_bias / abs(stripe_A_center.x - stripe_B_center.x));  //Difference between the stripe centers, biased by X (Larger constant means more bias)

	float area_difference_inv = 1.0 / fabs(stripe_B->area() - stripe_A->area()); 

	if (position_difference_inv > 0) { //If the distance is 0, it's the same object and that's not okay
		return position_difference_inv + area_difference_inv;
	} else {
		return 0; 
	}
}

bool StripeMatcher::FindPair (std::vector<cv::Rect*>* stripe_list, cv::Rect*& stripe_out_A, cv::Rect*& stripe_out_B) {
	//TODO: Make this slighly less naive
	float best_score = 0.0;
	if (stripe_list->size() > 1) {
		for (auto& stripe : *stripe_list) {
			for (auto& candidate : *stripe_list) {
				if (candidate != stripe) {
					float score = ScoreStripePair(stripe, candidate);
					if (score > best_score) {
						stripe_out_A = stripe;	
						stripe_out_B = candidate;	
						best_score = score;
					}
				}
			}
		}
	} else {
		return false;
	}
	std::cout << (stripe_out_A && stripe_out_B) << std::endl;
	return true;
}

StripeMatcher::~StripeMatcher() {

}
