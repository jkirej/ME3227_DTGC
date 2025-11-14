#include "image_processing.h"
#include "ocr.h"
#include "geometry.h"
#include "json_writer.h"
#include "llm_api_ollama.h"
#include "gcode_gen.h"
#include <iostream>
#include <fstream>

int main(int argc, char** argv){
    if(argc < 3){
        std::cout << "Usage: cad_to_gcode <input_image> <templates_json>\n";
        return 1;
    }
    std::string imgpath = argv[1];
    std::string templatefile = argv[2];


    std::cout << "Loading image...\n";
    ImageProcessor ip;
    if(!ip.loadImage(imgpath)){ std::cerr << "Failed to load image\n"; return 1; }
    
    std::cout << "Processing image...\n";
    ip.detectEdges();
    auto holes = ip.detectHoles();
    auto contours = ip.findContours();
    auto tboxes = ip.findTextBoxes();

    // debug overlay (save)
    ip.saveDebugOverlay("debug_overlay.png", holes, tboxes, contours);

    std::cout << "Running OCR...\n";
    OCR ocr;
    GeometryAssigner ga(100.0); // calibrate px_per_unit (pixels per inch or mm depending)
    std::vector<std::vector<cv::Point>> cpts;
    for(auto &ct : contours) cpts.push_back(ct.points);

    std::cout << "Associating features...\n";
    auto feats = ga.associate(holes, tboxes, contours, ocr);
    std::string features_json = JSONWriter::makeJSON(feats);
    std::cout << "Features JSON:\n" << features_json << "\n";

    std::cout << "Calling Ollama API...\n";
    std::cout << "Features JSON size: " << features_json.length() << " chars\n";
    OllamaAPI llm("qwen2.5-coder:1.5b", templatefile);
    std::string program_text, raw;
    bool ok = llm.requestProgram(features_json, program_text, raw);

    GCodeGen gen;
    if(!ok){
        std::cerr << "LLM request failed, using local generator.\n";
        gen.generateFromFeatures(feats, "output_fallback.nc");
        std::cout << "Wrote fallback G-code to output_fallback.nc\n";
    } else {
        if(program_text.find("ERROR - INVALID INPUT") != std::string::npos){
            std::cerr << "LLM reported invalid input. Check OCR / images. Using fallback generator.\n";
            gen.generateFromFeatures(feats, "output_fallback.nc");
            std::cout << "Wrote fallback G-code to output_fallback.nc\n";
        } else {
            // write LLM program text to file
            gen.writeProgramText(program_text, "output_from_llm.nc");
            std::cout << "Wrote LLM-generated G-code to output_from_llm.nc\n";
            
            // Also generate fallback for comparison
            gen.generateFromFeatures(feats, "output_fallback.nc");
            std::cout << "Also wrote fallback G-code to output_fallback.nc\n";
            
            // save raw response
            std::ofstream rf("llm_raw_response.json"); rf << raw; rf.close();
        }
    }
    return 0;
}

