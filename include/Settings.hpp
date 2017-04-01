//Make sure to keep sane defaults in here
class Settings {
	public:
		struct imgproc_settings_peg {
			int histogram_max = 50000;
			int histogram_min = 200;

			float histogram_percentile = 40.0;

			int morph_close = 5;
			int morph_open = 5;

			int sample_slicing_area_min = 2000;

			int matcher_bias_x = 1;
			int matcher_bias_y = 100;

			float median_filter_default = 0.0f;
			int median_filter_stack_size = 5;

			int 	spline_resolution = 300;
			float spline_wheel_radius = 0.1f;
			float spline_max_velocity = 0.1f;
			float spline_wheel_seperation = 0.2f;
			float spline_ctrlpt_distance = 1.0f;
		} imgproc_settings_peg_inst;

		struct server_options {
			int server_port = 5806;
		} server_options_inst;

		struct sensor_options_peg {
			int bgr_framerate = 60;
			int bgr_height = 480;
			int bgr_width = 640;
			int depth_framerate = 60;
			int depth_height = 360;
			int depth_width = 480;
			int exposure = 130;
			//char serial[11] = "2391016026";
			char serial[11] = "2391000767"; 
			//char serial[11] = "2391011471"; 
		} sensor_options_peg_inst;

		struct slider_limits_peg {
			int area_slider = 5000;
			int hue_slider_lower = 179;
			int hue_slider_upper = 179;
			int sat_slider_lower = 256;
			int sat_slider_upper = 256;
			int val_slider_lower = 256;
			int val_slider_upper = 256;
		} slider_limits_peg_inst;

		struct sliders_peg {
			int area_slider = 100;
			int hue_slider_lower = 44;
			int hue_slider_upper = 113;
			int sat_slider_lower = 140;
			int sat_slider_upper = 256;
			int val_slider_lower = 86;
			int val_slider_upper = 256;
		} sliders_peg_inst;
};