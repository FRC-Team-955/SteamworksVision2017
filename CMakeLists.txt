# Export all of the compile commands so I can use them in vim
SET( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

# Revision1 project
PROJECT(RS_Vision)

# Configure OpenCV and Puxixml packages (They have cmake release files)
FIND_PACKAGE(OpenCV REQUIRED)

# Manually the librealsense libraries and headers
FILE(GLOB Realsense_LIBS "/usr/local/lib/librealsense.so")
INCLUDE_DIRECTORIES("/usr/local/include/librealsense/")
#FILE(GLOB Pugixml_LIBS "/usr/lib/arm-linux-gnueabihf/libpugixml.so")
#INCLUDE_DIRECTORIES("/usr/include/pugixml.hpp")

# Include the include directory for the project 
INCLUDE_DIRECTORIES(include)

# Compile source files
FILE(GLOB SOURCES "src/*.cpp")

# Use c++11 and clang 
SET (CMAKE_CXX_COMPILER "/usr/bin/clang++")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ") 

# Make all of the sources executable
ADD_EXECUTABLE(RS_Vision ${SOURCES})

# Link libraries into the executable
TARGET_LINK_LIBRARIES(RS_Vision ${OpenCV_LIBS} ${Realsense_LIBS} ${Pugixml_LIBS} )
