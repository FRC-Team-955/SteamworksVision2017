# Steamworks Vision 2017
Team 955's vision software for SteamWorks. Currently work-in-progress.

If you're interested in running it, you'll need the following dependencies:
- Linux (With a kernel version that supports the R200 Realsense camera, probably >4.0?)
- CMake and GNU Make
- Intel's realsense library: [https://github.com/IntelRealSense/librealsense] () 
  - Also available on the AUR: [https://aur.archlinux.org/packages/librealsense/] () 
- Pugixml (For serialization of data)
- GNU Netcat (For network communication)
- (_Coming soon_) v4l-utils (Changing camera properties, such as exposure)

This should run on both desktop linux installations, and the Nvidia Jetson TK1 (At least when it's finished)

Contact duncan.freeman1@gmail.com if you have questions. Thank you!
