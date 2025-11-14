#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct DetectedHole {
    cv::Point2d center;
    double radius;
};

struct DetectedContour {
    std::vector<cv::Point> points;
};

struct TextBox {
    cv::Rect bbox;
    cv::Mat roi;
};

class ImageProcessor {
public:
    ImageProcessor() = default;
    bool loadImage(const std::string &path);
    void detectEdges();
    std::vector<DetectedHole> detectHoles();
    std::vector<DetectedContour> findContours();
    std::vector<TextBox> findTextBoxes();
    // debug draw
    void saveDebugOverlay(const std::string &path,
                         const std::vector<DetectedHole>& holes,
                         const std::vector<TextBox>& tboxes,
                         const std::vector<DetectedContour>& contours);
    cv::Mat src;
private:
    cv::Mat gray_, edges_;
};

