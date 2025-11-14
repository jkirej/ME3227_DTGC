#include "ocr.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <regex>
#include <stdexcept>

std::string OCR::runTesseract(const cv::Mat &roi){
    tesseract::TessBaseAPI tess;
    if(tess.Init(NULL, "eng")) {
        throw std::runtime_error("Tesseract init failed");
    }
    // convert ROI to 8-bit grayscale if needed
    cv::Mat im;
    if(roi.type() != CV_8UC1) cv::cvtColor(roi, im, cv::COLOR_BGR2GRAY);
    else im = roi;

    // write to Leptonica Pix
    Pix *pix = pixCreate(im.size().width, im.size().height, 8);
    for(int y=0;y<im.rows;y++){
        for(int x=0;x<im.cols;x++){
            uint8_t v = im.at<uint8_t>(y,x);
            pixSetPixel(pix, x, y, v);
        }
    }
    tess.SetImage(pix);
    tess.Recognize(0);
    char *out = tess.GetUTF8Text();
    std::string s = out ? std::string(out) : std::string();
    if(out) delete [] out;
    pixDestroy(&pix);
    tess.End();
    return s;
}

std::vector<std::string> OCR::extractNumbers(const std::string &text){
    std::vector<std::string> found;
    std::regex re(R"((\d+(\.\d+)?))");
    auto begin = std::sregex_iterator(text.begin(), text.end(), re);
    auto end = std::sregex_iterator();
    for(auto it = begin; it != end; ++it) found.push_back((*it).str());
    return found;
}

