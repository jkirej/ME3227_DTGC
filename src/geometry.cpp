#include "geometry.h"
#include <cmath>

GeometryAssigner::GeometryAssigner(double px_per_unit) : px_per_unit_(px_per_unit) {}
void GeometryAssigner::setScale(double px_per_unit){ px_per_unit_ = px_per_unit; }

std::vector<Feature> GeometryAssigner::associate(const std::vector<DetectedHole>& holes,
                                   const std::vector<TextBox>& tboxes,
                                   const std::vector<DetectedContour>& contours,
                                   OCR &ocr){
    std::vector<Feature> features;
    // holes -> match nearest text box, assume that is diameter if formatted as Ø or number
    // Limit to first 10 largest holes to avoid huge JSON
    std::vector<DetectedHole> filtered_holes = holes;
    std::sort(filtered_holes.begin(), filtered_holes.end(), [](const DetectedHole& a, const DetectedHole& b) {
        return a.radius > b.radius; // Sort by size, largest first
    });
    if(filtered_holes.size() > 10) filtered_holes.resize(10);
    
    for(auto &h : filtered_holes){
        double bestDist = 1e9; int bestIdx=-1;
        for(size_t i=0;i<tboxes.size();++i){
            cv::Point2d center(tboxes[i].bbox.x + tboxes[i].bbox.width/2.0, tboxes[i].bbox.y + tboxes[i].bbox.height/2.0);
            double d = cv::norm(center - h.center);
            if(d < bestDist){ bestDist = d; bestIdx = (int)i; }
        }
        Feature f; f.type="hole";
        f.x = round((h.center.x / px_per_unit_) * 1000.0) / 1000.0;  // Round to 3 decimal places
        f.y = round((h.center.y / px_per_unit_) * 1000.0) / 1000.0;
        f.diameter = round(((h.radius*2.0) / px_per_unit_) * 1000.0) / 1000.0;
        if(bestIdx != -1){
            std::cout << "  OCR on text box " << bestIdx << "..." << std::flush;
            std::string text = ocr.runTesseract(tboxes[bestIdx].roi);
            std::cout << " got: '" << text << "'" << std::endl;
            auto nums = ocr.extractNumbers(text);
            if(!nums.empty()){
                try { f.diameter = std::stod(nums[0]); }
                catch(...) {}
            }
        }
        features.push_back(f);
    }

    // Skip contours for now - focus on holes for LLM
    // TODO: Add simplified contour support later
    return features;
}

