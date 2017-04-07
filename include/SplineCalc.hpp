#ifndef SPLINECALC_HPP
#define SPLINECALC_HPP
#include <opencv2/opencv.hpp>
#include <tinysplinecpp.h>
#include <cmath>
#include <vector>
#include <MiscImgproc.hpp>
#include <Settings.hpp>

#define GENERATE_PLOT true

//Output CSV files for use in GNUPLOT
#if GENERATE_PLOT
#include <fstream>
#include <ostream>
#endif

#define PI 3.141592653589

using namespace MiscImgproc;

class SplineCalc {
	private:
		Settings::spline_generator_options* options;

#if GENERATE_PLOT
		bool plot_constructed = false;
		std::ofstream save_center_display; 			
		std::ofstream save_left_display; 	
		std::ofstream save_right_display; 	
		std::ofstream save_points_display; 	
#endif

		//Returns a position along the line that has a consistent distance (Adjustable by the time constant)
		float SplineChopRecurse (float max_travel, float start, float end, float tolerance, ts::BSpline* spline, int depth);

	public:
		SplineCalc(Settings::spline_generator_options* options);

		float ReallyCrappyRamp (float i); //TODO: Make this less shitty

		struct motion_plan_result {
			float compounded_distance, velocity, time_delta, spline_position;
			motion_plan_result (float compounded_distance, float velocity, float time_delta) {
				this->compounded_distance = compounded_distance;
				this->velocity = velocity;
				this->time_delta = time_delta;
			}
			motion_plan_result (float spline_position) {
				this->spline_position = spline_position;
			}
		};

		void CalcPaths(std::vector<motion_plan_result>* left_tracks, std::vector<motion_plan_result>* right_tracks, float goal_slope, cv::Point2f goal_position);

		cv::Point2f RationalVecConv(std::vector<ts::rational>* input);

		cv::Point2f RationalVecConv(std::vector<ts::rational> input);


};
#endif
