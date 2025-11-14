#pragma once
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include "image_processing.h"
#include "ocr.h"

struct Feature {
    std::string type; // "hole" or "contour"
    double x, y; // in units (inches)
    double diameter;
    std::vector<cv::Point> contour_pts; // used for contour features
};

class GeometryAssigner {
public:
    GeometryAssigner(double px_per_unit=100.0);
    void setScale(double px_per_unit);
    std::vector<Feature> associate(const std::vector<DetectedHole>& holes,
                                   const std::vector<TextBox>& tboxes,
                                   const std::vector<DetectedContour>& contours,
                                   OCR &ocr);
private:
    double px_per_unit_;
};

