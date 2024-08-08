#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <thread>
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include "../Sensors/IR_Sensor.cpp"

using namespace cv;

#define CAMFPS 15 //the fps

#define CAMWIDTH 640 //resolution
#define CAMHEIGHT 480

#define BELTSPEED 10

const int ROI[2][2]={{280,10},{450,470}}; //Region, of ineterest, 2 points defineing the square, in which the disk is searched for

enum color { black, white, colored };
typedef std::pair<double, enum color> diskInfo;
diskInfo ret(-1,white);
Piston piston;

enum color toEnum(cv::Scalar sc) 
{
    int avg = (sc[0] + sc[1] + sc[2]) / 3;
    if (abs(avg - sc[0]) > 15 || abs(avg - sc[1]) > 15 || abs(avg - sc[2]) > 15)
        return colored;
    if (avg < 100)
        return black;
    return white;
}

string colorName(color c)
{
    if (c == 0) return "black";
    if (c == 1) return "white";
    if (c == 2) return "RGB";
    return "";
}

cv::Scalar AverageColor(const cv::Mat& image, cv::Point center, int radius) 
{
    int avgColor[3] = { 0 }; // Initialize average color
    // Iterate through the circular region
    int count = 0;
    for (int y = std::max(center.y - radius, 0); y <= std::min(center.y + radius, image.rows - 1); y++) 
        for (int x = std::max(center.x - radius, 0); x <= std::min(center.x + radius, image.cols - 1); x++) 
            if (cv::norm(cv::Point(x, y) - center) <= radius) // Check if the pixel (x, y) is within the circle
            {
                avgColor[0] += image.at<cv::Vec3b>(y, x)[0];
                avgColor[1] += image.at<cv::Vec3b>(y, x)[1];
                avgColor[2] += image.at<cv::Vec3b>(y, x)[2];
                count++;
            }
    // Calculate average color
    if (count > 0) 
    {
        avgColor[0] /= count;
        avgColor[1] /= count;
        avgColor[2] /= count;
    }
    return cv::Scalar(avgColor[0], avgColor[1], avgColor[2]);
}

Point2f getCenter(std::vector<Point2f> v)
{
    Point2f avg(0,0);
    //get center
    for (int i=0; i < v.size(); i++) avg += v[i];
    avg.x /= v.size();
    avg.y /= v.size();
    return avg;
}

std::vector<Point2f> filter (std::vector<Point2f> &v, float distance, bool inplace=true)
{
    Point2f center = getCenter(v);
    std::vector<Point2f> ret;
    for(int i = 0; i < v.size(); i++)
        if(norm(v[i] - center)<=distance)
            ret.push_back(v[i]);
    if(inplace)
        v = ret;
    return ret;
}

inline unsigned int now() 
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

inline void wait(int ms) 
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

double match_score(const std::vector<DMatch>& matches)
{
    double total_score = 0.0;
    for (const auto& match : matches) 
        total_score += match.distance;
    double match_score = total_score / matches.size();
    if (100 < match_score) return 0;
    return 100 - match_score;
}
int camerrorcounter=0;
void camerror(){
    if(camerrorcounter>7){
    Logger::getInstance().log(CRITICAL, "The camera is obsecured! To continue, take care of that");
    camerrorcounter=0;
}
    camerrorcounter++;
}

void camera(int camera_index) 
{
    Mat obj_img = imread("disks/disk.PNG", IMREAD_GRAYSCALE);
    if (obj_img.empty()) 
    {
        throw std::runtime_error("Cannot open reference image");
        //return -1;
    }

    // Create ORB feature detector and descriptor extractor
    Ptr<ORB> detector = ORB::create();
    std::vector<KeyPoint> obj_keypoints;
    Mat obj_descriptors;

    // Detect keypoints and compute descriptors for the reference image
    detector->detectAndCompute(obj_img, noArray(), obj_keypoints, obj_descriptors);

    // Initialize webcam capture
    VideoCapture cap(camera_index);
    if (!cap.isOpened())
    {
        throw std::runtime_error("Cannot open webcam!"); 
        //return -1;
    }

    //set codec and resolution of capture
    cap.set(CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap.set(CAP_PROP_FRAME_WIDTH, CAMWIDTH);
    cap.set(CAP_PROP_FRAME_HEIGHT, CAMHEIGHT);

    // Create a brute-force matcher with Hamming distance
    BFMatcher matcher(NORM_HAMMING);
    
    //set up region of interest
    cv::Mat cmask = cv::Mat::zeros(cv::Size(CAMWIDTH, CAMHEIGHT), CV_8UC1);
    //cmask(cv::Rect(50, 50, CAMWIDTH - 100, CAMHEIGHT - 100))=1;
    cmask(cv::Rect(ROI[0][0], ROI[0][1], ROI[1][0]-ROI[0][0], ROI[1][1]-ROI[0][1])) = 1;
    
    int captured_frames=0, lastCamError=0;
    Mat frame, gray_frame;
    Logger::getInstance().log(INFO, "Initialisaton COMPLETE!");
    while (true) 
    {
        if (piston.pistonIsActive()) continue;
        int startT=now();
        cap >> frame; // Capture a frame from the webcam
        if (frame.empty()) 
        {
            throw std::runtime_error("Error captureing frame");
            //terminate();
        }
        
        // Convert the frame to grayscale
        cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
        // Detect keypoints and compute descriptors for the frame
        std::vector<KeyPoint> frame_keypoints;
        Mat frame_descriptors;
        detector->detectAndCompute(gray_frame, cmask, frame_keypoints, frame_descriptors);    
        // Match descriptors between the reference image and the frame
        std::vector<DMatch> matches;
        try
        {
            //it throws error if there isn't enough detail
            matcher.match(obj_descriptors, frame_descriptors, matches);
        }
        catch(...)
        {
            if(now() - lastCamError>1500)
            {
                camerror();
                lastCamError = now();
            }
            imshow("Object Detection", frame);
            waitKey(30);
            continue;
        }
        // Get the 35 best matchs
        std::sort(matches.begin(), matches.end());
        if(matches.size() > 20) matches.resize(20);

        // Extract location of matches
        std::vector<Point2f> obj_points, frame_points;
        for (size_t i = 0; i < matches.size(); i++) 
        {
            auto frame_point = frame_keypoints[matches[i].trainIdx].pt;
            auto obj_point = obj_keypoints[matches[i].queryIdx].pt;
            obj_points.push_back(obj_point);
            frame_points.push_back(frame_point);
        }
        filter(frame_points, CAMWIDTH / 3);
        if (matches.size() >= 15 && match_score(matches) > 55) 
        {
            for (int i = 0; i < frame_points.size(); i++)
                circle(frame, frame_points[i],4,(200, 200, 255),FILLED);
            circle(frame, getCenter(frame_points), 4, (255, 0, 0), FILLED);
            ret.first = getCenter(frame_points).y;
            ret.second = toEnum(AverageColor(frame, getCenter(frame_points), 40));
            if (ret.second != 0)
            {
                Logger::getInstance().log(INFO, "The camera detected a disk with color: " + colorName(ret.second));
                piston.SortDisk(ret.second);
            }
            
            //INTERUPT
        }
        
        //draws the region of interest
        line(frame, Point2i(ROI[0][0], ROI[0][1]), Point2f(ROI[1][0], ROI[0][1]), Scalar(0, 255, 0), 4);
        line(frame, Point2i(ROI[1][0], ROI[0][1]), Point2f(ROI[1][0], ROI[1][1]), Scalar(0, 255, 0), 4);
        line(frame, Point2i(ROI[1][0], ROI[1][1]), Point2f(ROI[0][0], ROI[1][1]), Scalar(0, 255, 0), 4);        
        line(frame, Point2i(ROI[0][0], ROI[1][1]), Point2f(ROI[0][0], ROI[0][1]), Scalar(0, 255, 0), 4);
        
        // Display the frame with detected object
        imshow("Object Detection", frame);
        captured_frames++;
        // Break the loop if the user presses 'q'/ regulates fps
        int time= ((1 / (double)CAMFPS) * 1000.0) -(now() - startT);
        if (time < 10) time = 10;
        if (waitKey(time) >= 0) //DO NOT DELETE OR CHANGE IT TO ANY OTHER WAIT FUNCTION
            break;
    }
    //std::cout << captured_frames << std::endl;
}

void inc() 
{
    while (true) 
    {
        wait(10);
        if (ret.first >= 0)
            ret.first = ret.first - BELTSPEED;
    }
}

inline void CamInit(int camera_index)
{
    std::thread t(camera,camera_index);
    t.detach();
    std::thread s(inc);
    s.detach();
}

inline diskInfo getCam() 
{
    return ret;
}
