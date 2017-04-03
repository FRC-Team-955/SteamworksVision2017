#ifndef SPLINECALC_HPP
#define SPLINECALC_HPP
#include <opencv2/opencv.hpp>
#include <tinysplinecpp.h>
#include <cmath>
#include <vector>
#include <MiscImgproc.hpp>

#define GENERATE_DEBUG true

//Output CSV files for use in GNUPLOT
#if GENERATE_DEBUG
#include <fstream>
#include <ostream>
#endif

#define PI 3.141592653589

using namespace MiscImgproc;

class SplineCalc {
	private:
		int resolution = 0;
		float wheel_radius = 0.1f;
		float wheel_seperation = 0.0f;
		float wheel_circumference = 0.0f;
		float max_velocity = 0.0f;
		float ctrlpt_distance = 0.0f;
		float step = 0.0f;
		bool already_generated = false;

		float delta_time = 0.0f;

		cv::Point2f robot_origin = cv::Point2f(0.0f, 0.0f);
		cv::Point2f end_offset = cv::Point2f(0.0f, 0.0f);

#if GENERATE_DEBUG
		std::ofstream save_center_display; 			
		std::ofstream save_left_display; 	
		std::ofstream save_right_display; 	
		std::ofstream save_points_display; 	
#endif

		float SplineChopRecurse (float max_travel, float start, float end, float tolerance, ts::BSpline* spline);

	public:
		SplineCalc(
				int resolution, 
				float wheel_radius, 
				float max_velocity, float 
				wheel_seperation, 
				float ctrlpt_distance, 
				float delta_time, 
				cv::Point2f end_offset);


		float ReallyCrappyRamp (float i);

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
