#include <PegFinder.hpp>

PegFinder::PegFinder(VideoInterface* video_interface,
		SaveEntry *sliders_save				,	
		SaveEntry *sliders_limits			,	
		SaveEntry *imgproc_save				,	
		SaveEntry *application_options	,	
		SaveEntry *video_interface_save	
		) {

	this->video_interface = video_interface;
	this->sliders_save				=	sliders_save		 	; 
	this->sliders_limits				=	sliders_limits		 	; 	
	this->imgproc_save				=	imgproc_save		 	; 
	this->application_options		=	application_options	; 
	this->video_interface_save		=	video_interface_save	;

	//TODO: Ewwww

	//Object initialization
	histogram_goal_center = new Histogram((*imgproc_save)["histogram_min"], (*imgproc_save)["histogram_max"]);
	hist_inner_roi_left = new Histogram((*imgproc_save)["histogram_min"], (*imgproc_save)["histogram_max"]);
	hist_inner_roi_right = new Histogram((*imgproc_save)["histogram_min"], (*imgproc_save)["histogram_max"]);

	matcher = new StripeMatcher (1, 100);

	distance_median = new Median<float>(5,0);
	angle_median = new Median<float>(5,0);

	morph_open_struct_element = getStructuringElement(MORPH_RECT, Size( 2*(*imgproc_save)["morph_open"] + 1, 2*(*imgproc_save)["morph_open"]+1 ), Point( (*imgproc_save)["morph_open"], (*imgproc_save)["morph_open"] ) ); //Make sure that objects have a certain area
}

std::string PegFinder::ProcessFrame() {
	std::string ret;
	video_interface->GrabFrames();
	if ((*application_options)["show_depth"]) {
		imshow("DEPTH", *video_interface->largeDepthCV * 6);
	}

	video_interface->bgrmatCV->copyTo(display_buffer);

	cvtColor (*video_interface->bgrmatCV, raw_hsv_color, COLOR_BGR2HSV); // Convert to HSV colorspace
	inRange (raw_hsv_color,
			Scalar ((*sliders_save)["hue_slider_lower"], (*sliders_save)["sat_slider_lower"], (*sliders_save)["val_slider_lower"]),
			Scalar ((*sliders_save)["hue_slider_upper"], (*sliders_save)["sat_slider_upper"], (*sliders_save)["val_slider_upper"]),
			hsv_range_mask);
	morphologyEx(hsv_range_mask, hsv_range_mask_filtered, MORPH_OPEN, morph_open_struct_element);


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
			rectangle(display_buffer, *bounding_rectangle, Scalar(0, 255, 255), 2);

			//Create a new stripe object
			stripes.push_back(bounding_rectangle); 

		} else {
			rectangle(display_buffer, *bounding_rectangle, Scalar(0, 0, 255), 2);
		}
	}

	// Find the best pair (Find closest by most similar Y value, area, and distance to adjacent)
	Rect* left_stripe = nullptr;
	Rect* right_stripe = nullptr;
	if (matcher->FindPair(&stripes, left_stripe, right_stripe) && left_stripe && right_stripe) {
		if (GetCenter(right_stripe).x > GetCenter(left_stripe).x) {
			std::swap(left_stripe, right_stripe);
		}

		Rect goal_center_Rect = Rect(
				Point(left_stripe->br().x, left_stripe->tl().y), 
				Point(right_stripe->tl().x, right_stripe->br().y)); //Only get the inner area between the two strips because the strips themselves reflect the IR that allows for depth (See notes) 
		Rect left_hist_portion_Rect  = goal_center_Rect; 
		Rect right_hist_portion_Rect = goal_center_Rect; 

		//TODO: Move these constants to variables in the save file
		left_hist_portion_Rect.x  -= left_hist_portion_Rect.width  * 1; //Needs to be 1 more widths farther than the right one because the edge starts from the x position (left edge), not the centers
		right_hist_portion_Rect.x += right_hist_portion_Rect.width * 1;

		left_hist_portion_Rect.y  -= left_hist_portion_Rect.height  * 3; //Needs to be 1 more widths farther than the right one because the edge starts from the x position (left edge), not the centers
		right_hist_portion_Rect.y -= right_hist_portion_Rect.height * 3;

		if (left_hist_portion_Rect.y < 0) {
			left_hist_portion_Rect.y = 0;
		}

		if (right_hist_portion_Rect.y < 0) {
			right_hist_portion_Rect.y = 0;
		}

		if ((*application_options)["show_overlays"]) {
			rectangle(display_buffer, goal_center_Rect, Scalar(255, 0, 0), 2);  
			line(display_buffer, GetCenter(left_stripe), GetCenter(right_stripe), Scalar(0, 0, 255), 3, CV_AA); 
		}

		rectangle(display_buffer, left_hist_portion_Rect, Scalar(255, 0, 255), 2);  
		rectangle(display_buffer, right_hist_portion_Rect, Scalar(255,0, 255), 2);  

		histogram_goal_center->insert_histogram_data(&goal_center_Rect, video_interface->largeDepthCV);
		hist_inner_roi_left->insert_histogram_data(&left_hist_portion_Rect, video_interface->largeDepthCV);
		hist_inner_roi_right->insert_histogram_data(&right_hist_portion_Rect, video_interface->largeDepthCV);

		//TODO: Move this to the save class instead
		stream_doc.reset();
		pugi::xml_node root_node = stream_doc.append_child("Root");

		int depth = histogram_goal_center->take_percentile((*imgproc_save)["histogram_percentile"]);

		int depth_left_Rect = hist_inner_roi_left->take_percentile((*imgproc_save)["histogram_percentile"]);
		int depth_right_Rect = hist_inner_roi_right->take_percentile((*imgproc_save)["histogram_percentile"]);

		int x_center_left_Rect 	= MidPoint(left_hist_portion_Rect.tl(),  left_hist_portion_Rect.br()).x;
		int x_center_right_Rect = MidPoint(right_hist_portion_Rect.tl(), right_hist_portion_Rect.br()).x;


		int distance_to_screen_edge = (*video_interface_save)["bgr_width"] / 2;

		//Eight and a quarter inches is how far apart the centers of the stripes should be, by spec
		int eight_and_quarter_inches_in_px = PointDistance(GetCenter(left_stripe), GetCenter(right_stripe));
		float px_per_inch = (float)eight_and_quarter_inches_in_px / 8.25; 

		//Left and right center x positions in inches 
		float x_center_left_Rect_inch		 = x_center_left_Rect	 / px_per_inch;
		float x_center_right_Rect_inch	 = x_center_right_Rect / px_per_inch;

		//Left and right center depths in inches
		float depth_left_Rect_inch 	= 	(float)depth_left_Rect 	* 0.0393701f;
		float depth_right_Rect_inch 	= 	(float)depth_right_Rect * 0.0393701f;

		float depth_x_slope = (float)(depth_right_Rect_inch - depth_left_Rect_inch) / (float)(x_center_right_Rect_inch - x_center_left_Rect_inch); 

		int magnitude_x_px = MidPoint(GetCenter(left_stripe), GetCenter(right_stripe)).x - distance_to_screen_edge;
		float magnitude_x_inch = magnitude_x_px / px_per_inch;

		float angle = (atanf(depth_x_slope) * 180.0f) / PI;

		//TODO: Time stamping!
		if (depth > 0 && depth_left_Rect_inch > 0 && depth_right_Rect_inch > 0) {
			distance_median->insert_median_data((depth_left_Rect_inch + depth_left_Rect_inch) / 2);
			angle_median->insert_median_data(angle);
			root_node.append_attribute("distance_to_target") = distance_median->compute_median();
			root_node.append_attribute("angle") = angle_median->compute_median(); 
			//root_node.append_attribute("slope_depth") = depth_x_slope;
			//root_node.append_attribute("pixels_per_inch_at_depth") = eight_and_quarter_inches_in_px; 
			//root_node.append_attribute("slope_height") = (float)(GetCenter(stripe_A).y - GetCenter(stripe_B).y) / (float)(GetCenter(stripe_A).x - GetCenter(stripe_B).x);
			//root_node.append_attribute("x_magnitude_inch") = magnitude_x_inch; 
			//stream_doc.save(*output_ss); 
			//TODO: Yuck
			std::stringstream ss;
			stream_doc.save(ss);
			ret = ss.str();
			//*output_ss << "timestamp: " << video_interface<< std::endl;
		}
//		std::cout << "angle: " << angle << std::endl;
//		std::cout << " x left center offset inch: " << x_center_left_Rect_inch << std::endl;
//		std::cout << "  x right center offset inch: " << x_center_right_Rect_inch << std::endl;
//		std::cout << "   center x offsets: " << fabs(x_center_right_Rect_inch - x_center_left_Rect_inch) << std::endl;
//		std::cout << "    x left center depth inch: " << depth_left_Rect << std::endl;
//		std::cout << "     x right center depth inch: " << depth_right_Rect << std::endl;
//		std::cout << "      center depth offsets: " << fabs(depth_right_Rect_inch - depth_left_Rect_inch) << std::endl;
//		std::cout << "       depth_x_slope: " << depth_x_slope << std::endl;
//		std::cout << "        mag x px: " << magnitude_x_inch << std::endl;
//
	}

	//Get a box that encapsulates both stripes

	// Find the most promising pair (Closest, most centered)

	//TODO: Make another mat to display on instead of writing over the video_interface's mat
	if ((*application_options)["show_rgb"]) {
		imshow("COLOR", display_buffer);
	}
	return ret;
}

PegFinder::~PegFinder() {
	delete[] histogram_goal_center;
	delete[] hist_inner_roi_left;
	delete[] hist_inner_roi_right;
}
