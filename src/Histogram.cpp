#include "Histogram.hpp"

void Histogram::clear_histogram() {
	memset (histogram, 0, sizeof (unsigned short) * range);
	data_sum = 0;
}


Histogram::Histogram (int min, int max) {
	this->min = min;
	this->max = max;
	range = max - min;
	histogram = new unsigned short [range];
	clear_histogram();
}

void Histogram::insert_histogram_data (unsigned short* data, size_t data_length) {
	clear_histogram();

	for (size_t i = 0; i < data_length; i++) {
		if (data[i] >= min && data[i] <= max) {
			histogram[data[i] - min]++;
			data_sum ++ ;
		}
	}

}

void Histogram::insert_histogram_data (cv::Rect* image_ROI, cv::Mat* image) {
	clear_histogram();
	unsigned short pixel = 0;
	for (size_t x = image_ROI->x; x < image_ROI->width + image_ROI->x; x++) {
		for (size_t y = image_ROI->y; y < image_ROI->height + image_ROI->y; y++) {
			pixel = image->at<unsigned short> (y, x); 
			if (pixel >= min && pixel <= max) {
				histogram[pixel - min]++;
				data_sum++;
			}
		}
	}
}

unsigned short Histogram::take_percentile (int percentile) {
	unsigned short* histogram_start_copy = histogram;
	unsigned long accumulator = 0;
	int percentile_max = ceil ( (float) percentile / 100.0f * (float) data_sum);
	size_t bin_number = 0;

	std::cout << "MAX: " << percentile_max << std::endl;

	while (accumulator < percentile_max) {
		accumulator += *histogram_start_copy;
		histogram_start_copy++;
		bin_number++;
	}

	if (bin_number > 0) {
		return bin_number - 1 + min;
	} else {
		return 0;
	}
}

Histogram::~Histogram() {
	delete[] histogram;
}
