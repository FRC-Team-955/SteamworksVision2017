# Steamworks Vision 2017
Team 955's vision software for SteamWorks. Currently work-in-progress.

If you're interested in running it, you'll need the following dependencies:
- Linux (With a kernel version that supports the R200 Realsense camera, probably >4.0?)
- CMake and GNU Make
- OpenCV - Open source computer vision library: [http://opencv.org/] ()
- Intel's realsense library: [https://github.com/IntelRealSense/librealsense] () 
  - Also available on the AUR: [https://aur.archlinux.org/packages/librealsense/] () 
- Pugixml (For serialization of data that java can understand): [https://github.com/zeux/pugixml] ()
- TinySpline [https://github.com/msteinbeck/tinyspline] () - TODO: Fix so that we don't have to use the '39f3f5e5c1a79e41f13a2a27d55533dc19c34321' version. If you want to build the project as of 
now you'll need that specific commit. This needs to be updated.

This should run on both desktop linux installations, and the Nvidia Jetson TK1, see [http://www.jetsonhacks.com/2016/06/20/intel-realsense-camera-installation-nvidia-jetson-tk1/] () for info on how to get the tegra to work with an R200. You'll need the R21.3 version of NVIDIA's software for this to work; [https://developer.nvidia.com/linux-tegra-r213] ()

Contact duncan.freeman1@gmail.com if you have questions. Thank you!
