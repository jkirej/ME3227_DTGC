#include "json_writer.h"
#include "json.hpp"
using nlohmann::json;

std::string JSONWriter::makeJSON(const std::vector<Feature>& features, const std::string& units){
    json j;
    j["units"] = units;
    j["features"] = json::array();
    for(auto &f : features){
        json jf;
        jf["type"] = f.type;
        jf["x"] = f.x;
        jf["y"] = f.y;
        jf["diameter"] = f.diameter;
        if(!f.contour_pts.empty()){
            jf["contour"] = json::array();
            for(auto &p : f.contour_pts){
                jf["contour"].push_back({{"x", p.x / 1.0}, {"y", p.y / 1.0}});
            }
        }
        j["features"].push_back(jf);
    }
    return j.dump(2);
}

