# Steamworks Vision 2017
Team 955's vision software for SteamWorks. Currently work-in-progress.

If you're interested in running it, you'll need the following dependencies:
- Linux (With a kernel version that supports the R200 Realsense camera, probably >4.0?)
- CMake and GNU Make
- OpenCV - Open source computer vision library: [http://opencv.org/] ()
- Intel's realsense library: [https://github.com/IntelRealSense/librealsense] () 
  - Also available on the AUR: [https://aur.archlinux.org/packages/librealsense/] () 
- Pugixml (For serialization of data that java can understand)
- XPra, a way to have a virtual X desktop: [http://xpra.org/] ()
- A copy of JSONCPP is included by default, just so you don't have to download it (It doesn't have to have a specific version or anything)

This should run on both desktop linux installations, and the Nvidia Jetson TK1, see [http://www.jetsonhacks.com/2016/06/20/intel-realsense-camera-installation-nvidia-jetson-tk1/] () for info on how to get the tegra to work with an R200. You'll need the R21.3 version of NVIDIA's software for this to work; [https://developer.nvidia.com/linux-tegra-r213] ()

Contact duncan.freeman1@gmail.com if you have questions. Thank you!
