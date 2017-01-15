#include "Median.hpp"

Median::Median (size_t max_stack_length, double default_value) {
	this->max_stack_length = max_stack_length;
	this->default_value = default_value;
}

void Median::insert_median_data (unsigned short data) {
	median_stack.push_front (data);

	if (median_stack.size() > max_stack_length) {
		median_stack.pop_back();
	}
}

unsigned short Median::compute_median () {							
	if (median_stack.size() != 0)
	{
		std::deque<unsigned short> median_stack_copy = median_stack;
		std::sort (median_stack_copy.begin(), median_stack_copy.end());
		return median_stack_copy[median_stack_copy.size() / 2];
	} 
	else 
	{
		return (int)default_value;
	}
}

Median::~Median() {

}
