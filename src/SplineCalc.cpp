#include <SplineCalc.hpp>

SplineCalc::SplineCalc(int resolution, float wheel_radius, float max_velocity, float wheel_seperation, float ctrlpt_distance, float time_unit_multiplier) {
	this->resolution = resolution;
	this->wheel_radius = wheel_radius;
	this->wheel_seperation = wheel_seperation;
	this->max_velocity = max_velocity;
	this->ctrlpt_distance = ctrlpt_distance;
	this->time_unit_multiplier = time_unit_multiplier;
	step = 1.0f / (float)resolution;

#if GENERATE_DEBUG
	save_center_display.open("/tmp/center_display.csv");
	save_left_display.open("/tmp/left_display.csv");
	save_right_display.open("/tmp/right_display.csv");
	save_points_display.open("/tmp/points_display.csv"); 
#endif
}

cv::Point2f SplineCalc::RationalVecConv(std::vector<ts::rational>* input) {
	return cv::Point2f(input->at(0), input->at(1));
}

cv::Point2f SplineCalc::RationalVecConv(std::vector<ts::rational> input) {
	return cv::Point2f(input.at(0), input.at(1));
}

//TODO: Make this less shitty
float SplineCalc::ReallyCrappyRamp (float i) {
	float first_point = 0.1f;
	float second_point = 0.9f;
	if (i > 0.0f && i < first_point) {
		return (i / first_point) * max_velocity;
	} else if (i >= first_point && i <= second_point) {
		return max_velocity;
	} else if (i >= second_point && i < 0.999) {
		return (1.0f - ((i - second_point) / (1.0f - second_point))) * max_velocity;
	} else {
		return 0.01f;
	}
}

void SplineCalc::CalcPaths(std::vector<motion_plan_result>* left_tracks, std::vector<motion_plan_result>* right_tracks, float goal_slope, cv::Point2f goal_position) {
	//Create spline and control points
	ts::BSpline spline(3, 2, 6, TS_CLAMPED);

	std::vector<ts::rational> ctrlp = spline.ctrlp();
	//First three are for the robot's position and vector
	ctrlp[0] =  robot_origin.x;  				// x0
	ctrlp[1] =  robot_origin.y;  				// y0

	ctrlp[2] =  robot_origin.x;  				// x1
	ctrlp[3] =  robot_origin.y + ctrlpt_distance; 				// y1

	ctrlp[4] =  robot_origin.x;  				// x2
	ctrlp[5] =  robot_origin.y + (ctrlpt_distance * 2.0f);  	// y2

	//Last three are for the goal's position and vector
	ctrlp[6] =  goal_position.x;  			// x3
	ctrlp[7] =  goal_position.y;  			// y3

	cv::Point2f first_outcrop = MoveAlongLine(goal_slope < 0, ctrlpt_distance, NegativeReciprocal(goal_slope), goal_position);

	ctrlp[8] = first_outcrop.x;  				// x4
	ctrlp[9] = first_outcrop.y;  				// y4

	cv::Point2f second_outcrop = MoveAlongLine(goal_slope < 0, ctrlpt_distance * 2.0f, NegativeReciprocal(goal_slope), goal_position);

	ctrlp[10] =  second_outcrop.x;  			// x5
	ctrlp[11] =  second_outcrop.y;  			// y5

	spline.setCtrlp(ctrlp);

	//Derive the curve, as we use the slope at each point (really it's perpendicular) to find the point for each track
	ts::BSpline derivation = spline.derive();

	cv::Point2f last_position_left (0.0f, 0.0f);
	cv::Point2f last_position_right (0.0f, 0.0f);
	cv::Point2f last_position_center (0.0f, 0.0f);

	float compounded_left = 0.0f;
	float compounded_right = 0.0f;

	for (float i = 0.0f; i < 1.0f - step; i += step) {
		cv::Point2f spline_center = RationalVecConv(spline.evaluate(i).result());
		cv::Point2f spline_derive = RationalVecConv(derivation.evaluate(i).result());

#if GENERATE_DEBUG
		save_center_display << spline_center.x << "," << spline_center.y << std::endl;
#endif

		/* 
		 * Create new points that start out at the point along the spline,
		 * and extend out along the normal the length of the offset from 
		 * the center of the drive base
		 */
		cv::Point2f normal_left = MoveAlongLine(spline_derive.y > 0,
				wheel_seperation,
				NegativeReciprocal(spline_derive.y / spline_derive.x),
				spline_center
				);

		cv::Point2f normal_right = MoveAlongLine(spline_derive.y < 0,
				wheel_seperation,
				NegativeReciprocal(spline_derive.y / spline_derive.x),
				spline_center
				);

		if (i == 0.0f) {
			last_position_left = normal_left;
			last_position_right = normal_right;
			last_position_center = spline_center;
		}

		float travel_center = PointDistance(last_position_center, spline_center);
		float travel_left = PointDistance(last_position_left, normal_left);
		float travel_right = PointDistance(last_position_right, normal_right);

		//TODO: Max velocity at ramp point!
		float time_delta = (travel_left > travel_right ? travel_left : travel_right) / ReallyCrappyRamp(i);

		//TODO: Add in the wheel circumference when trevor is ready for it
		compounded_left += travel_left;
		compounded_right += travel_right;

#if GENERATE_DEBUG
		save_left_display 	<< normal_left.x 	<< ", " << normal_left.y	<< ", " << travel_left / time_delta << std::endl;
		save_right_display 	<< normal_right.x << ", " << normal_right.y	<< ", " << travel_right / time_delta << std::endl;
#endif

		float velocity_left = (time_delta != 0 ? travel_left / time_delta : 0);
		float velocity_right = (time_delta != 0 ? travel_right / time_delta : 0);

		left_tracks->push_back(
				motion_plan_result(
					compounded_left,
					velocity_left,
					time_delta * time_unit_multiplier
					)
				);

		right_tracks->push_back(
				motion_plan_result(
					compounded_right,
					velocity_right,
					time_delta * time_unit_multiplier
					)
				);

		last_position_left = normal_left;
		last_position_right = normal_right;
		last_position_center = spline_center;
	}

#if GENERATE_DEBUG
	for (int i = 0; i < ctrlp.size(); i+=2) {
		save_points_display << ctrlp[i] << "," << ctrlp[i+1] << std::endl;
	}
#endif

}
