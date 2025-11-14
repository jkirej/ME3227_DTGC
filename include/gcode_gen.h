#pragma once
#include "geometry.h"
#include <string>
#include <vector>

class GCodeGen {
public:
    GCodeGen(double retract=0.1, double feed=12.0);
    // Generate from local features (fallback) and write .nc
    void generateFromFeatures(const std::vector<Feature>& feats, const std::string &out_path);
    // Save LLM program text (if LLM returned program_text)
    void writeProgramText(const std::string &program_text, const std::string &out_path);
private:
    double retract_;
    double feed_;
};

