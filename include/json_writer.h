#pragma once
#include "geometry.h"
#include <string>
#include <vector>

class JSONWriter {
public:
    static std::string makeJSON(const std::vector<Feature>& features, const std::string& units="inch");
};

