#include "image_processing.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

bool ImageProcessor::loadImage(const std::string &path){
    src = imread(path, IMREAD_COLOR);
    if(src.empty()) return false;
    cvtColor(src, gray_, COLOR_BGR2GRAY);
    return true;
}

void ImageProcessor::detectEdges(){
    Mat bl;
    GaussianBlur(gray_, bl, Size(5,5), 1.5);
    Canny(bl, edges_, 50, 150);
}

std::vector<DetectedHole> ImageProcessor::detectHoles(){
    std::vector<DetectedHole> holes;
    std::vector<Vec3f> circles;
    HoughCircles(gray_, circles, HOUGH_GRADIENT, 1, 30, 100, 30, 10, 80);
    for(auto &c : circles){
        DetectedHole h;
        h.center = Point2d(c[0], c[1]);
        h.radius = c[2];
        holes.push_back(h);
    }
    return holes;
}

std::vector<DetectedContour> ImageProcessor::findContours(){
    std::vector<DetectedContour> out;
    Mat thr;
    threshold(gray_, thr, 200, 255, THRESH_BINARY_INV);
    std::vector<std::vector<Point>> contours;
    cv::findContours(thr, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    for(auto &ct : contours){
        DetectedContour d;
        d.points = ct;
        out.push_back(d);
    }
    return out;
}

std::vector<TextBox> ImageProcessor::findTextBoxes(){
    Mat thr;
    adaptiveThreshold(gray_, thr, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 41, 10);
    Mat labels, stats, centroids;
    int n = connectedComponentsWithStats(thr, labels, stats, centroids, 8, CV_32S);
    std::vector<TextBox> boxes;
    for(int i=1;i<n;i++){
        int area = stats.at<int>(i, CC_STAT_AREA);
        int w = stats.at<int>(i, CC_STAT_WIDTH);
        int h = stats.at<int>(i, CC_STAT_HEIGHT);
        if(area < 60 || w < 6 || h < 6) continue;
        Rect r(stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP), w, h);
        if(r.width > 300 || r.height > 120) continue;
        TextBox tb;
        tb.bbox = r;
        Mat roi = gray_(r).clone();
        tb.roi = roi;
        boxes.push_back(tb);
    }
    return boxes;
}

void ImageProcessor::saveDebugOverlay(const std::string &path,
                         const std::vector<DetectedHole>& holes,
                         const std::vector<TextBox>& tboxes,
                         const std::vector<DetectedContour>& contours){
    Mat out = src.clone();
    for(auto &h: holes){
        circle(out, h.center, (int)h.radius, Scalar(0,0,255), 2);
        circle(out, h.center, 2, Scalar(0,255,0), -1);
    }
    for(auto &tb: tboxes){
        rectangle(out, tb.bbox, Scalar(255,0,0), 2);
    }
    for(auto &ct: contours){
        drawContours(out, std::vector<std::vector<Point>>{ct.points}, -1, Scalar(0,255,255), 2);
    }
    imwrite(path, out);
}

