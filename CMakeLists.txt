# Export all of the compile commands so I can use them in vim
SET( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

# Revision1 project
PROJECT(RS_Vision)

# Configure OpenCV and Pugixml packages (They have cmake release files)
FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
#FIND_PACKAGE(PugiXML REQUIRED)

# Add the -lpthread flag instead of just the libs (Usefile for nvim), only on CMAKE 3.0+
#set(THREADS_PREFER_PTHREAD_FLAG ON)

find_path (PUGIXML_INCLUDE_DIR NAMES pugixml.hpp PATHS ${PUGIXML_HOME}/include)
find_library (PUGIXML_LIBRARY NAMES pugixml PATHS ${PUGIXML_HOME}/lib)

# Manually the librealsense libraries and headers
FILE(GLOB Realsense_LIBS "/usr/local/lib/librealsense.so")
INCLUDE_DIRECTORIES("/usr/local/include/librealsense/")
#FILE(GLOB Pugixml_LIBS "/usr/lib/arm-linux-gnueabihf/libpugixml.so")
#INCLUDE_DIRECTORIES("/usr/include/pugixml.hpp")

# Include the include directory for the project 
INCLUDE_DIRECTORIES(include ${PUGIXML_INCLUDE_DIR}/pugixml.hpp)
MESSAGE(STATUS "Pugixml include dir: " ${PUGIXML_INCLUDE_DIR})

# Compile source files
FILE(GLOB SOURCES "src/*.cpp")

# Use c++11 and clang 
SET (CMAKE_CXX_COMPILER "/usr/bin/clang++")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ") 

# Make all of the sources executable
ADD_EXECUTABLE(RS_Vision ${SOURCES})

# Link libraries into the executable
TARGET_LINK_LIBRARIES(RS_Vision ${OpenCV_LIBS} ${Realsense_LIBS} ${PUGIXML_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
