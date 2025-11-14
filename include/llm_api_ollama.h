#pragma once
#include <string>

class OllamaAPI {
public:
    OllamaAPI(const std::string& model_name, const std::string& template_file);

    // Send features JSON + templates to Ollama and receive G-code
    bool requestProgram(
        const std::string& features_json,
        std::string& program_text,
        std::string& raw_response
    );

private:
    std::string model_;
    std::string templates_json_;

    std::string loadFile(const std::string& path);
};

