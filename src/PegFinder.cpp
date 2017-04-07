#include <PegFinder.hpp>

PegFinder::PegFinder(Settings* sf) {
	this->sf = sf;
	histogram_goal_center	= new Histogram<unsigned short>(sf->imgproc_settings_peg_inst.histogram_min, sf->imgproc_settings_peg_inst.histogram_max);
	hist_inner_roi_left	 	= new Histogram<unsigned short>(sf->imgproc_settings_peg_inst.histogram_min, sf->imgproc_settings_peg_inst.histogram_max);
	hist_inner_roi_right 	= new Histogram<unsigned short>(sf->imgproc_settings_peg_inst.histogram_min, sf->imgproc_settings_peg_inst.histogram_max);

	matcher = new StripeMatcher (sf->imgproc_settings_peg_inst.matcher_bias_x, sf->imgproc_settings_peg_inst.matcher_bias_y);

	distance_median 			= new Median<float>(sf->imgproc_settings_peg_inst.median_filter_stack_size,sf->imgproc_settings_peg_inst.median_filter_default);
	angle_median 				= new Median<float>(sf->imgproc_settings_peg_inst.median_filter_stack_size,sf->imgproc_settings_peg_inst.median_filter_default);
	slope_median 				= new Median<float>(sf->imgproc_settings_peg_inst.median_filter_stack_size,sf->imgproc_settings_peg_inst.median_filter_default);
	target_x_offset_median	= new Median<float>(sf->imgproc_settings_peg_inst.median_filter_stack_size,sf->imgproc_settings_peg_inst.median_filter_default);

	depth_left_median_mm 	= new Median<int>(sf->imgproc_settings_peg_inst.median_filter_stack_size,sf->imgproc_settings_peg_inst.median_filter_default);
	depth_right_median_mm	= new Median<int>(sf->imgproc_settings_peg_inst.median_filter_stack_size,sf->imgproc_settings_peg_inst.median_filter_default);

	morph_open_struct_element = getStructuringElement(MORPH_RECT, sf->imgproc_settings_peg_inst.morph_open_size, sf->imgproc_settings_peg_inst.morph_open_anchor ); 
	morph_close_struct_element = getStructuringElement(MORPH_RECT, sf->imgproc_settings_peg_inst.morph_close_size, sf->imgproc_settings_peg_inst.morph_close_anchor ); 
}

void PegFinder::ProcessFrame(Mat* depth_image, Mat* color_image, Mat* display_buffer, imgproc_results* results) {
#if DEBUG_SHOW_DEPTH
		imshow("DEPTH", *depth_image * 6);
#endif

	color_image->copyTo(*display_buffer);

	cvtColor (*color_image, raw_hsv_color, COLOR_BGR2HSV); // Convert to HSV colorspace
	inRange (raw_hsv_color,
			Scalar (sf->sliders_peg_inst.hue_slider_lower, sf->sliders_peg_inst.sat_slider_lower, sf->sliders_peg_inst.val_slider_lower),
			Scalar (sf->sliders_peg_inst.hue_slider_upper, sf->sliders_peg_inst.sat_slider_upper, sf->sliders_peg_inst.val_slider_upper),
			hsv_range_mask);
	morphologyEx(hsv_range_mask, hsv_range_mask_filtered, MORPH_OPEN, morph_open_struct_element);
	morphologyEx(hsv_range_mask_filtered, hsv_range_mask_filtered, MORPH_CLOSE, morph_close_struct_element);

#if DEBUG_SHOW_HSV
		imshow("HSV_RANGE_SELECT", hsv_range_mask_filtered);
#endif

	// Find viable contours
	findContours(hsv_range_mask_filtered, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0,0));
	for (auto& stripe : stripes) {
		delete[] stripe;
	}
	stripes.clear();
	for (auto& contour : contours) {

		Rect* bounding_rectangle = new Rect(boundingRect(contour));		

		Point bounding_rectangle_center = Point(bounding_rectangle->width / 2, bounding_rectangle->height / 2) + bounding_rectangle->tl();

		float rectangle_dim_ratio = (float)bounding_rectangle->width / (float)bounding_rectangle->height;	

		//Check if it fits contour criteria (Area, and ROI width to height ratio)
		if ( bounding_rectangle->area() >= sf->sliders_peg_inst.area_slider && ToleranceCheck(rectangle_dim_ratio, sf->imgproc_settings_peg_inst.stripe_dimension_ratio, sf->imgproc_settings_peg_inst.stripe_dimension_ratio_tolerance) ) {
			rectangle(*display_buffer, *bounding_rectangle, Scalar(0, 255, 255), 2);

			//Create a new stripe object
			stripes.push_back(bounding_rectangle); 

		} else {
			rectangle(*display_buffer, *bounding_rectangle, Scalar(0, 0, 255), 2);
		}
	}

	// Find the best pair (Find closest by most similar Y value, area, and distance to adjacent)
	Rect* left_stripe = nullptr;
	Rect* right_stripe = nullptr;

	if (!stripes.size()) {
		results->stripes_found = 0;
	}

	if (matcher->FindPair(&stripes, left_stripe, right_stripe) && left_stripe && right_stripe) {
		results->stripes_found = 2;
		if (GetCenter(right_stripe).x > GetCenter(left_stripe).x) {
			std::swap(left_stripe, right_stripe);
		}

		Rect goal_center_Rect = Rect(
				Point(left_stripe->br().x, left_stripe->tl().y), 
				Point(right_stripe->tl().x, right_stripe->br().y)); //Only get the inner area between the two strips because the strips themselves reflect the IR that allows for depth (See notes) 
		Rect left_hist_portion_Rect  = goal_center_Rect; 
		Rect right_hist_portion_Rect = goal_center_Rect; 

		//TODO: Move these constants to variables in the save file, document more
		left_hist_portion_Rect.x  -= left_hist_portion_Rect.width  * 1; //Needs to be 1 more widths farther than the right one because the edge starts from the x position (left edge), not the centers
		right_hist_portion_Rect.x += right_hist_portion_Rect.width * 1;

		left_hist_portion_Rect.y  -= left_hist_portion_Rect.height  * 2; //Needs to be 2 more widths farther than the right one because the edge starts from the x position (left edge), not the centers
		right_hist_portion_Rect.y -= right_hist_portion_Rect.height * 2;

		//Cut off the side of the box when it hits the edge instead of trying to sample outside of the image (That breaks things)
		if (left_hist_portion_Rect.y < 0) {
			left_hist_portion_Rect.y = 0;
		}

		if (right_hist_portion_Rect.y < 0) {
			right_hist_portion_Rect.y = 0;
		}

		if (left_hist_portion_Rect.x < 0) {
			int cutoff = abs(left_hist_portion_Rect.x);
			left_hist_portion_Rect.x = 0;
			left_hist_portion_Rect.width -= cutoff;
		}

		if (right_hist_portion_Rect.x + right_hist_portion_Rect.width > sf->sensor_options_peg_inst.bgr_width) {
			int cutoff = abs((right_hist_portion_Rect.x + right_hist_portion_Rect.width) - sf->sensor_options_peg_inst.bgr_width);
			right_hist_portion_Rect.width -= cutoff;
		}

		//If cutting of the side of the box makes it too small, move it to the other side

		//{"sample_slicing_area_min"	,	10		}
		if (left_hist_portion_Rect.area() > (sf->imgproc_settings_peg_inst.sample_slicing_area_min ^ 2) || right_hist_portion_Rect.area() > (sf->imgproc_settings_peg_inst.sample_slicing_area_min ^ 2)) {
			if (left_hist_portion_Rect.area() < (sf->imgproc_settings_peg_inst.sample_slicing_area_min ^ 2)) {
				left_hist_portion_Rect = right_hist_portion_Rect; //Copy it, then slice both into their respective halves
				left_hist_portion_Rect.width /= 2;
				right_hist_portion_Rect.width /= 2;

				right_hist_portion_Rect.x += left_hist_portion_Rect.width;
			}

			if (right_hist_portion_Rect.area() < (sf->imgproc_settings_peg_inst.sample_slicing_area_min ^ 2)) {
				right_hist_portion_Rect = left_hist_portion_Rect; //Copy it, then slice both into their respective halves
				left_hist_portion_Rect.width /= 2;
				right_hist_portion_Rect.width /= 2;

				right_hist_portion_Rect.x += left_hist_portion_Rect.width;
			}

#if DEBUG_SHOW_OVERLAYS
			rectangle(*display_buffer, goal_center_Rect, Scalar(255, 0, 0), 2);  
			line(*display_buffer, GetCenter(left_stripe), GetCenter(right_stripe), Scalar(0, 0, 255), 3, CV_AA); 
			rectangle(*display_buffer, left_hist_portion_Rect, Scalar(255, 0, 255), 2);  
			rectangle(*display_buffer, right_hist_portion_Rect, Scalar(255,0, 255), 2);  
#endif

			histogram_goal_center->insert_histogram_data(&goal_center_Rect, depth_image);
			hist_inner_roi_left->insert_histogram_data(&left_hist_portion_Rect, depth_image);
			hist_inner_roi_right->insert_histogram_data(&right_hist_portion_Rect, depth_image);

			//TODO: Document the hell out of this, and reorganize (Class?)
			int depth_left_Rect = hist_inner_roi_left->take_percentile(sf->imgproc_settings_peg_inst.histogram_percentile);
			int depth_right_Rect = hist_inner_roi_right->take_percentile(sf->imgproc_settings_peg_inst.histogram_percentile);
			depth_right_Rect += -10; //TODO: HORRIBLE MAGIC NUMBER - AT LEAST ADD IT TO SETTINGS
			depth_left_median_mm->insert_median_data(depth_left_Rect); //Make new variables for these
			depth_right_median_mm->insert_median_data(depth_right_Rect);
			depth_left_Rect = depth_left_median_mm->compute_median(); 
			depth_right_Rect = depth_right_median_mm->compute_median(); 
			std::cout << "Left: " << depth_left_Rect << " Right: " << depth_right_Rect << std::endl;

			int x_center_left_Rect 	= MidPoint(left_hist_portion_Rect.tl(),  left_hist_portion_Rect.br()).x;
			int x_center_right_Rect = MidPoint(right_hist_portion_Rect.tl(), right_hist_portion_Rect.br()).x;


			int distance_to_screen_edge = sf->sensor_options_peg_inst.bgr_width / 2;

			//Eight and a quarter inches is how far apart the centers of the stripes should be, by spec
			int eight_and_quarter_inches_in_px = PointDistance(GetCenter(left_stripe), GetCenter(right_stripe));
			float px_per_inch = (float)eight_and_quarter_inches_in_px / 8.25; 

			//Left and right center x positions in inches 
			float x_center_left_Rect_inch		 = x_center_left_Rect	 / px_per_inch;
			float x_center_right_Rect_inch	 = x_center_right_Rect / px_per_inch;

			//Left and right center depths in inches
			float depth_left_Rect_inch 	= 	(float)depth_left_Rect 	* 0.0393701f;
			float depth_right_Rect_inch 	= 	(float)depth_right_Rect * 0.0393701f;

			//Slope from one side of the goal to the other as alighned too our plane
			float depth_x_slope = (float)(depth_right_Rect_inch - depth_left_Rect_inch) / (float)(x_center_right_Rect_inch - x_center_left_Rect_inch); 

			int magnitude_x_px = MidPoint(GetCenter(left_stripe), GetCenter(right_stripe)).x - distance_to_screen_edge;
			float magnitude_x_inch = magnitude_x_px / px_per_inch;

			float angle = (atanf(depth_x_slope) * 180.0f) / PI;

			results->x_offset_to_target = magnitude_x_inch;
			if (depth_left_Rect_inch > 0 && depth_right_Rect_inch > 0) {
				results->distance_found = true;
				distance_median->insert_median_data((depth_left_Rect_inch + depth_left_Rect_inch) / 2);
				angle_median->insert_median_data(angle);
				slope_median->insert_median_data(depth_x_slope);
				target_x_offset_median->insert_median_data(magnitude_x_inch);
				results->distance_to_target = distance_median->compute_median();
				results->angle_to_target = angle_median->compute_median(); 
				results->slope_to_target = slope_median->compute_median();
				results->target_x_offset = target_x_offset_median->compute_median();
			} else {
				results->distance_found = false;
			}
		} else {
			//std::cerr << "Both tapes are too small when cut!" << std::endl;
			results->stripes_found = 1;
			results->stripe_width  = (left_hist_portion_Rect.width + right_hist_portion_Rect.width) / 2;
		}
	}  
}

PegFinder::~PegFinder() {
	delete[] histogram_goal_center;
	delete[] hist_inner_roi_left;
	delete[] hist_inner_roi_right;
}
