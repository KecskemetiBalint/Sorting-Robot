cmake_minimum_required(VERSION 3.10)
project(EmbSysGR10)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include directories from OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Create an executable
add_executable(${PROJECT_NAME} camera2/camera.cpp)

# Link OpenCV libraries 
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})