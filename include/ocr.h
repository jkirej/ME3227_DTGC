#pragma once
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

class OCR {
public:
    OCR() {}
    std::string runTesseract(const cv::Mat &roi);
    std::vector<std::string> extractNumbers(const std::string &text);
};

