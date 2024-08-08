#!/bin/bash
g++ -fdiagnostics-color=always main.cpp -o b -lopencv_core -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lopencv_calib3d -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lpthread -lwiringPi -I/usr/include/opencv4/
