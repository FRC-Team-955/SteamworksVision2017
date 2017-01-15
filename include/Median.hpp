#ifndef MEDIAN_HPP
#define MEDIAN_HPP
#include <cmath>
#include <stdio.h>
#include <deque>
#include <algorithm>

class Median
{
	private:
		std::deque<unsigned short> median_stack;
		size_t max_stack_length;
		unsigned short default_value;
		
	public:
		Median (size_t max_stack_length, double default_value);
		
		void insert_median_data (unsigned short data);
		
		unsigned short compute_median ();
		
		~Median();
};
#endif
