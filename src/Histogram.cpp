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
			//data_sum += data[i];
			data_sum ++ ;
		}
	}

}

unsigned short Histogram::take_percentile (int percentile) {
	unsigned short* histogram_start_copy = histogram;
	unsigned long accumulator = 0;
	int percentile_max = ceil ( (float) percentile / 100.0f * (float) data_sum);
	size_t bin_number = 0;

	while (accumulator < percentile_max) {
		accumulator += *histogram_start_copy;
		histogram_start_copy++;
		bin_number++;
	}

	//if (bin_number - 1 + min >= 0) {
	return bin_number - 1 + min;
	//} else {
	//	return 0;
	//}
}

Histogram::~Histogram() {
	delete[] histogram;
}
