#include <PegFinder.hpp>

PegFinder::PegFinder(Realsense* sensor, std::ostream* output_ss, std::unordered_map<std::string, std::unordered_map<std::string, int>*>* saved_fields ) {
	this->output_ss = output_ss;
	this->sensor = sensor;

	sliders_save				= (*saved_fields)["Sliders"				];						
	imgproc_save				= (*saved_fields)["Imgproc_const"		];		
	application_options		= (*saved_fields)["Application_Options"];		
	sensor_save	 				= (*saved_fields)["Sensor"					];	 

	//Object initialization
	hist_roi = new Histogram((*imgproc_save)["histogram_min"], (*imgproc_save)["histogram_max"]);
	hist_inner_roi_left = new Histogram((*imgproc_save)["histogram_min"], (*imgproc_save)["histogram_max"]);
	hist_inner_roi_right = new Histogram((*imgproc_save)["histogram_min"], (*imgproc_save)["histogram_max"]);

	morph_open_struct_element = getStructuringElement(MORPH_RECT, Size( 2*(*imgproc_save)["morph_open"] + 1, 2*(*imgproc_save)["morph_open"]+1 ), Point( (*imgproc_save)["morph_open"], (*imgproc_save)["morph_open"] ) ); //Make sure that objects have a certain area
}

void PegFinder::ProcessFrame() {

	sensor->GrabFrames();
	if ((*application_options)["show_depth"]) {
		imshow("DEPTH", *sensor->largeDepthCV * 6);
	}

	cvtColor (*sensor->rgbmatCV, raw_hsv_color, COLOR_BGR2HSV); // Convert to HSV colorspace
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

		Rect bounding_rectangle = boundingRect(contour);		

		Point bounding_rectangle_center = Point(bounding_rectangle.width / 2, bounding_rectangle.height / 2) + bounding_rectangle.tl();

		float rectangle_dim_ratio = (float)bounding_rectangle.width / (float)bounding_rectangle.height;	

		//Check if it fits contour criteria (Area, and ROI width to height ratio)
		if ( bounding_rectangle.area() >= (*sliders_save)["area_slider"] && ToleranceCheck(rectangle_dim_ratio, 2.0/5.0, 0.3) ) {
			rectangle(*sensor->bgrmatCV, bounding_rectangle, Scalar(0, 255, 255), 2);

			//Create a new stripe object
			stripe_object* stripe = new stripe_object;
			stripe->ROI = bounding_rectangle;
			stripe->center = bounding_rectangle_center;
			stripes.push_back(stripe); 

		} else {
			rectangle(*sensor->bgrmatCV, bounding_rectangle, Scalar(0, 0, 255), 2);
		}
	}

	// Find the best pair (Find closest by most similar Y value, area, and distance to adjacent)
	float best_score = 0.0;
	stripe_object* best_stripe = nullptr;
	stripe_object* best_stripe_pair = nullptr;
	if (stripes.size() > 1) {
		for (auto& stripe : stripes) {
			for (auto& candidate : stripes) {
				if (candidate != stripe) {
					float score = candidate->ScorePair(stripe);
					if (score > best_score) {
						best_stripe = stripe;	
						best_stripe_pair = candidate;	
						best_score = score;
					}
				}
			}
		}
		if (best_stripe && best_stripe_pair) {
			Rect left = best_stripe->ROI;
			Rect right = best_stripe_pair->ROI;
			if (left.x > right.x) {
				std::swap(left, right);
			}

			Rect final_ROI = Rect(
					Point(left.br().x, left.tl().y), 
					Point(right.tl().x, right.br().y)); //Only get the inner area between the two strips because the strips themselves reflect the IR that allows for depth (See notes) 
			Rect left_hist_portion_ROI = Rect(
					final_ROI.tl(), 
					Point(final_ROI.tl().x + (final_ROI.width / 3), final_ROI.br().y)); //Get the left 1/3 of the ROI for use in slope calculations
			Rect right_hist_portion_ROI = Rect(
					Point(final_ROI.tl().x + (2 * final_ROI.width / 3), final_ROI.tl().y), //Get the right 1/3 of the ROI for use in slope calculations
					final_ROI.br());

			if ((*application_options)["show_overlays"]) {
				rectangle(*sensor->bgrmatCV, final_ROI, Scalar(255, 0, 0), 2);  
				line(*sensor->bgrmatCV, best_stripe->center, best_stripe_pair->center, Scalar(0, 0, 255), 3, CV_AA); 
				//putText(*sensor->bgrmatCV, std::to_string(best_score), best_stripe_pair-> center, CV_FONT_HERSHEY_TRIPLEX, 5, Scalar(0, 0, 255)); 
			}

			rectangle(*sensor->bgrmatCV, left_hist_portion_ROI, Scalar(255, 0, 255), 2);  
			rectangle(*sensor->bgrmatCV, right_hist_portion_ROI, Scalar(255, 0, 255), 2);  

			//Prepare the pixel lists for the histogram classes
			unsigned short* pixelList = new unsigned short[final_ROI.area()];
			unsigned short *moving_pixelList = pixelList; //A pointer that gets changed, so we copy the start value
			for (int x = final_ROI.tl().x; x < final_ROI.br().x; x++) {
				for (int y = final_ROI.tl().y; y < final_ROI.br().y; y++) {
					*moving_pixelList = sensor->largeDepthCV->at<unsigned short> (y, x);
					moving_pixelList++;
				}
			}
			hist_roi->insert_histogram_data(pixelList, final_ROI.area());

			//Prepare the pixel lists for the histogram classes
			unsigned short* pixelList_left_ROI = new unsigned short[left_hist_portion_ROI.area()];
			unsigned short *moving_pixelList_left_ROI = pixelList_left_ROI; //A pointer that gets changed, so we copy the start value
			for (int x = left_hist_portion_ROI.tl().x; x < left_hist_portion_ROI.br().x; x++) {
				for (int y = left_hist_portion_ROI.tl().y; y < left_hist_portion_ROI.br().y; y++) {
					*moving_pixelList_left_ROI = sensor->largeDepthCV->at<unsigned short> (y, x);
					moving_pixelList_left_ROI++;
				}
			}
			hist_inner_roi_left->insert_histogram_data(pixelList_left_ROI, left_hist_portion_ROI.area());

			//Prepare the pixel lists for the histogram classes
			unsigned short* pixelList_right_ROI = new unsigned short[final_ROI.area()];
			unsigned short *moving_pixelList_right_ROI = pixelList_right_ROI; //A pointer that gets changed, so we copy the start value
			for (int x = final_ROI.tl().x; x < final_ROI.br().x; x++) {
				for (int y = final_ROI.tl().y; y < final_ROI.br().y; y++) {
					*moving_pixelList_right_ROI = sensor->largeDepthCV->at<unsigned short> (y, x);
					moving_pixelList_right_ROI++;
				}
			}
			hist_inner_roi_right->insert_histogram_data(pixelList_right_ROI, final_ROI.area());

			//TODO: Move this to the save class instead
			stream_doc.reset();
			pugi::xml_node root_node = stream_doc.append_child("Root");

			int depth = hist_roi->take_percentile((*imgproc_save)["histogram_percentile"]);

			int depth_left_ROI = hist_inner_roi_left->take_percentile((*imgproc_save)["histogram_percentile"]);
			int depth_right_ROI = hist_inner_roi_right->take_percentile((*imgproc_save)["histogram_percentile"]);

			int x_center_left_ROI = MidPoint(left_hist_portion_ROI.tl(), left_hist_portion_ROI.br()).x;
			int x_center_right_ROI = MidPoint(right_hist_portion_ROI.tl(), right_hist_portion_ROI.br()).x;

			float depth_x_slope = (float)(depth_right_ROI - depth_left_ROI) / (float)(x_center_right_ROI - x_center_left_ROI); 

			int distance_to_screen_edge = (*sensor_save)["bgr_width"] / 2;

			int eight_and_quarter_inches_in_px = PointDistance(&best_stripe->center, &best_stripe_pair->center);
			float px_per_inch = (float)eight_and_quarter_inches_in_px / 8.25; 

			int magnitude_x_px = MidPoint(best_stripe->center, best_stripe_pair->center).x - distance_to_screen_edge;
			float magnitude_x_inch = magnitude_x_px / px_per_inch;

			//TODO: Time stamping!
			if (depth > 0) {
				root_node.append_attribute("distance_to_target") = depth; 
				root_node.append_attribute("slope_depth") = depth_x_slope;
				root_node.append_attribute("pixels_per_inch_at_depth") = eight_and_quarter_inches_in_px; 
				root_node.append_attribute("slope_height") = (float)(best_stripe->center.y - best_stripe_pair->center.y) / (float)(best_stripe->center.x - best_stripe_pair->center.x);
				root_node.append_attribute("x_magnitude_inch") = magnitude_x_inch; 
				stream_doc.save(*output_ss); //TODO: Set to named pipe later
			}
			delete[] pixelList;
			delete[] pixelList_left_ROI;
			delete[] pixelList_right_ROI;
		}
	}

	//Get a box that encapsulates both stripes

	// Find the most promising pair (Closest, most centered)

	//TODO: Make another mat to display on instead of writing over the sensor's mat
	if ((*application_options)["show_rgb"]) {
		imshow("COLOR", *sensor->bgrmatCV);
	}

	waitKey(10);


}
