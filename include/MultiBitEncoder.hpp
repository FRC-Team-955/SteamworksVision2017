#ifndef MULTIBITENCODER
#define MULTIBITENCODER
#include <pthread.h>
#include <vector>
#include <opencv2/opencv.hpp>

class MultiBitEncoder {
	private:	
		typedef union {
			short int_var;
			char ch1ch2[2];
		} COMBO;

		struct range_arguments {
			size_t begin;
			size_t end;
		};

		struct thread_attributes {
			range_arguments* range;
			cv::Mat* input_16UC1; 
			cv::Mat* output_8UC3;
			pthread_t* thread;
		};

		std::vector<thread_attributes*> threads;

		cv::Mat* input_16UC1; 
		cv::Mat* output_8UC3;

		int thread_count;

		static void* EncoderThread (void* arg);



	public:
		MultiBitEncoder(int thread_count, cv::Mat* input_16UC1, cv::Mat* output_8UC3);

		void Encode16Bit ();
};
#endif
