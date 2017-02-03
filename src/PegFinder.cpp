#include <PegFinder.hpp>

//TODO: Make the biases part of the config node
float PegFinder::ScoreStripePair (Rect* stripe_A, Rect* stripe_B, float x_bias, float y_bias) {
	Point stripe_A_center = (stripe_A->tl() + stripe_A->br()) / 2;
	Point stripe_B_center = (stripe_B->tl() + stripe_B->br()) / 2;

	float position_difference_inv =  //Biased position difference score (Bigger is worse)
		(y_bias / abs(stripe_A_center.y - stripe_B_center.y)) + //Difference between the stripe centers, biased by Y (Larger constant means more bias)
		(x_bias / abs(stripe_A_center.x - stripe_B_center.x));  //Difference between the stripe centers, biased by X (Larger constant means more bias)

	float area_difference_inv = 1.0 / fabs(stripe_B->area() - stripe_A->area()); 

	if (position_difference_inv > 0) { //If the distance is 0, it's the same object and that's not okay
		return position_difference_inv + area_difference_inv;
	} else {
		return 0; 
	}
}

PegFinder::PegFinder(VideoInterface* video_interface, std::unordered_map<std::string, std::unordered_map<std::string, int>*>* saved_fields ) {
	this->video_interface = video_interface;

	//TODO: Ewwww
	sliders_save				= (*saved_fields)["Sliders"					];						
	imgproc_save				= (*saved_fields)["Imgproc_const"			];		
	application_options		= (*saved_fields)["Application_Options"	];		
	video_interface_save	 	= (*saved_fields)["Sensor"						];	 

	//Object initialization
	histogram_goal_center = new Histogram((*imgproc_save)["histogram_min"], (*imgproc_save)["histogram_max"]);
	hist_inner_roi_left = new Histogram((*imgproc_save)["histogram_min"], (*imgproc_save)["histogram_max"]);
	hist_inner_roi_right = new Histogram((*imgproc_save)["histogram_min"], (*imgproc_save)["histogram_max"]);

	morph_open_struct_element = getStructuringElement(MORPH_RECT, Size( 2*(*imgproc_save)["morph_open"] + 1, 2*(*imgproc_save)["morph_open"]+1 ), Point( (*imgproc_save)["morph_open"], (*imgproc_save)["morph_open"] ) ); //Make sure that objects have a certain area
}

std::string PegFinder::ProcessFrame() {
	std::string ret;
	video_interface->GrabFrames();
	if ((*application_options)["show_depth"]) {
		imshow("DEPTH", *video_interface->largeDepthCV * 6);
	}

	cvtColor (*video_interface->rgbmatCV, raw_hsv_color, COLOR_BGR2HSV); // Convert to HSV colorspace
	//TODO: Implement depth-hsv combination select. See notes/classification for more detail
	inRange (raw_hsv_color,
			Scalar ((*sliders_save)["hue_slider_lower"], (*sliders_save)["sat_slider_lower"], (*sliders_save)["val_slider_lower"]),
			Scalar ((*sliders_save)["hue_slider_upper"], (*sliders_save)["sat_slider_upper"], (*sliders_save)["val_slider_upper"]),
			hsv_range_mask);
	morphologyEx(hsv_range_mask, hsv_range_mask_filtered, MORPH_OPEN, morph_open_struct_element);


	//TODO: Save file selections of outputs
	if ((*application_options)["show_HSV"]) {
		imshow("HSV_RANGE_SELECT", hsv_range_mask_filtered);
	}

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
		//TODO: Set the tolerance from the save file!
		if ( bounding_rectangle->area() >= (*sliders_save)["area_slider"] && ToleranceCheck(rectangle_dim_ratio, 2.0/5.0, 0.2) ) {
			rectangle(*video_interface->bgrmatCV, *bounding_rectangle, Scalar(0, 255, 255), 2);

			//Create a new stripe object
			stripes.push_back(bounding_rectangle); 

		} else {
			rectangle(*video_interface->bgrmatCV, *bounding_rectangle, Scalar(0, 0, 255), 2);
		}
	}

	// Find the best pair (Find closest by most similar Y value, area, and distance to adjacent)
	float best_score = 0.0;
	Rect* stripe_A = nullptr;
	Rect* stripe_B = nullptr;
	if (stripes.size() > 1) {
		for (auto& stripe : stripes) {
			for (auto& candidate : stripes) {
				if (candidate != stripe) {
					float score = ScoreStripePair(stripe, candidate, 1.0, 100.0);
					if (score > best_score) {
						stripe_A = stripe;	
						stripe_B = candidate;	
						best_score = score;
					}
				}
			}
		}
		if (stripe_A && stripe_B) {
			Rect* left = stripe_A;
			Rect* right = stripe_B;
			if (GetCenter(stripe_A).x > GetCenter(stripe_B).x) {
				std::swap(left, right);
			}

			Rect goal_center_Rect = Rect(
					Point(left->br().x, left->tl().y), 
					Point(right->tl().x, right->br().y)); //Only get the inner area between the two strips because the strips themselves reflect the IR that allows for depth (See notes) 
			Rect left_hist_portion_Rect  = goal_center_Rect; 
			Rect right_hist_portion_Rect = goal_center_Rect; 

			//TODO: Move these constants to variables in the save file
			left_hist_portion_Rect.x  -= left_hist_portion_Rect.width  * 5;
			right_hist_portion_Rect.x += right_hist_portion_Rect.width * 5;

			left_hist_portion_Rect.width  += left_hist_portion_Rect.width  * 1;
			right_hist_portion_Rect.width += right_hist_portion_Rect.width * 1;

			if ((*application_options)["show_overlays"]) {
				rectangle(*video_interface->bgrmatCV, goal_center_Rect, Scalar(255, 0, 0), 2);  
				line(*video_interface->bgrmatCV, GetCenter(stripe_A), GetCenter(stripe_B), Scalar(0, 0, 255), 3, CV_AA); 
				//putText(*video_interface->bgrmatCV, std::to_string(best_score), stripe_B-> center, CV_FONT_HERSHEY_TRIPLEX, 5, Scalar(0, 0, 255)); 
			}

			rectangle(*video_interface->bgrmatCV, left_hist_portion_Rect, Scalar(255, 0, 255), 2);  
			rectangle(*video_interface->bgrmatCV, right_hist_portion_Rect, Scalar(255, 0, 255), 2);  

			histogram_goal_center->insert_histogram_data(&goal_center_Rect, video_interface->largeDepthCV);
			histogram_goal_center->insert_histogram_data(&left_hist_portion_Rect, video_interface->largeDepthCV);
			histogram_goal_center->insert_histogram_data(&right_hist_portion_Rect, video_interface->largeDepthCV);

			//TODO: Move this to the save class instead
			stream_doc.reset();
			pugi::xml_node root_node = stream_doc.append_child("Root");

			int depth = histogram_goal_center->take_percentile((*imgproc_save)["histogram_percentile"]);

			int depth_left_Rect = hist_inner_roi_left->take_percentile((*imgproc_save)["histogram_percentile"]);
			int depth_right_Rect = hist_inner_roi_right->take_percentile((*imgproc_save)["histogram_percentile"]);

			int x_center_left_Rect 	= MidPoint(left_hist_portion_Rect.tl(),  left_hist_portion_Rect.br()).x;
			int x_center_right_Rect = MidPoint(right_hist_portion_Rect.tl(), right_hist_portion_Rect.br()).x;

			int distance_to_screen_edge = (*video_interface_save)["bgr_width"] / 2;

			int eight_and_quarter_inches_in_px = PointDistance(GetCenter(stripe_A), GetCenter(stripe_B));
			float px_per_inch = (float)eight_and_quarter_inches_in_px / 8.25; 

			//Left and right center x positions in inches 
			float x_center_left_Rect_inch = x_center_left_Rect / px_per_inch;
			float x_center_right_Rect_inch = x_center_right_Rect / px_per_inch;

			//Left and right center depths in inches
			float depth_left_Rect_inch = (float)depth_left_Rect * 0.0393701f;
			float depth_right_Rect_inch = (float)depth_right_Rect * 0.0393701f;

			float depth_x_slope = (float)(depth_right_Rect_inch - depth_left_Rect_inch) / (float)(x_center_right_Rect_inch - x_center_left_Rect_inch); 

			int magnitude_x_px = MidPoint(GetCenter(stripe_A), GetCenter(stripe_B)).x - distance_to_screen_edge;
			float magnitude_x_inch = magnitude_x_px / px_per_inch;

			float angle = (atanf(depth_x_slope) * 180.0f) / PI;

			//TODO: Time stamping!
			if (depth > 0 && depth_left_Rect_inch > 0 && depth_right_Rect_inch > 0) {
				//root_node.append_attribute("distance_to_target") = depth; 
				//root_node.append_attribute("slope_depth") = depth_x_slope;
				//root_node.append_attribute("pixels_per_inch_at_depth") = eight_and_quarter_inches_in_px; 
				//root_node.append_attribute("slope_height") = (float)(GetCenter(stripe_A).y - GetCenter(stripe_B).y) / (float)(GetCenter(stripe_A).x - GetCenter(stripe_B).x);
				//root_node.append_attribute("x_magnitude_inch") = magnitude_x_inch; 
				//root_node.append_attribute("angle") = angle; 
				//stream_doc.save(*output_ss); 
				std::cout << "angle: " << angle << std::endl;
				std::cout << " x left center offset inch: " << x_center_left_Rect_inch << std::endl;
				std::cout << "  x right center offset inch: " << x_center_right_Rect_inch << std::endl;
				std::cout << "   center x offsets: " << fabs(x_center_right_Rect_inch - x_center_left_Rect_inch) << std::endl;
				std::cout << "    x left center depth inch: " << depth_left_Rect_inch << std::endl;
				std::cout << "     x right center depth inch: " << depth_right_Rect_inch << std::endl;
				std::cout << "      center depth offsets: " << fabs(depth_right_Rect_inch - depth_left_Rect_inch) << std::endl;
				std::cout << "       depth_x_slope: " << depth_x_slope << std::endl;
				//TODO: Yuck
				std::stringstream ss;
				stream_doc.save(ss);
				ret = ss.str();
				//*output_ss << "timestamp: " << video_interface<< std::endl;
			}
		}
	}

	//Get a box that encapsulates both stripes

	// Find the most promising pair (Closest, most centered)

	//TODO: Make another mat to display on instead of writing over the video_interface's mat
	if ((*application_options)["show_rgb"]) {
		imshow("COLOR", *video_interface->bgrmatCV);
	}
	return ret;
}

PegFinder::~PegFinder() {
	delete[] histogram_goal_center;
	delete[] hist_inner_roi_left;
	delete[] hist_inner_roi_right;
}
