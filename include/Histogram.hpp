#ifndef HISTOGRAM_HPP
#define HISTOGRAM_HPP
#include <cmath>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <opencv2/opencv.hpp>

class Histogram
{
	private:
		unsigned short* histogram;
		size_t range;
		unsigned long data_sum = 0;

		void clear_histogram();
		
	public:
		size_t min, max;
		Histogram (int min, int max);
		
		void insert_histogram_data (unsigned short* data, size_t data_length); 

		void insert_histogram_data (cv::Rect* image_ROI, cv::Mat* image); 
		
		unsigned short take_percentile (int percentile);

		~Histogram();
		
};
#endif
