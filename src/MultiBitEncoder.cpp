#include <MultiBitEncoder.hpp>

MultiBitEncoder::MultiBitEncoder(int thread_count, cv::Mat* input_16UC1, cv::Mat* output_8UC3) {
	this->thread_count = thread_count;	
	this->input_16UC1 = input_16UC1;
	this->output_8UC3 = output_8UC3;
	//TODO: Add assertions that these are pointers to the right types
}

void* MultiBitEncoder::EncoderThread (void* arg) {
	thread_attributes* attrib = (thread_attributes*)arg;
	range_arguments* range = attrib->range;

	//std::cout << "Start: " << range->begin << " End: " << range->end << std::endl;
	unsigned char* output_8UC3_copy = attrib->output_8UC3->data;
	unsigned short* input_16UC1_copy = (unsigned short*)attrib->input_16UC1->data;

	COMBO pixel;
	for (size_t i = range->begin; i < range->end; i++) {
		pixel.int_var = *input_16UC1_copy++;
		*output_8UC3_copy++ = pixel.ch1ch2[0];
		*output_8UC3_copy++ = pixel.ch1ch2[1];
		output_8UC3_copy++; 
	}

	//delete[] range;
	//delete[] attrib;

	std::cout << "Thread finished" << std::endl;
	return NULL;
}

void MultiBitEncoder::Encode16Bit () {
	threads.clear();
	size_t length = input_16UC1->size().area();
	size_t stride = length / thread_count;
	size_t data_position = 0;
	for (int i = 0; i < thread_count; i++) {
		thread_attributes* attrib = new thread_attributes();			
		attrib->input_16UC1 = input_16UC1;
		attrib->output_8UC3 = output_8UC3;
		pthread_t* thread = new pthread_t();
		attrib->thread = thread;

		range_arguments* range = new range_arguments();
		range->begin = data_position;
		data_position += stride;
		if (data_position > length) { data_position = length; }
		range->end = data_position;
		data_position++; //Make sure you don't collide with the next thread

		attrib->range = range;

		threads.push_back(attrib);
		pthread_create(attrib->thread, NULL, EncoderThread, attrib);
	}

	std::cout << "Wating for threads (" << threads.size() << ")..." << std::endl;
	for (auto& thread : threads) {
		pthread_join(*thread->thread, NULL);
		delete[] thread->range;
		delete[] thread->thread;
	}

	std::cout << "All threads joined" << std::endl;
}


