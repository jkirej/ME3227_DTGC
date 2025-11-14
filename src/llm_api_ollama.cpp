#include "llm_api_ollama.h"
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string OllamaAPI::loadFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) return "";
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

OllamaAPI::OllamaAPI(const std::string& model_name, const std::string& template_file)
    : model_(model_name)
{
    templates_json_ = loadFile(template_file);
    if (templates_json_.empty()) {
        std::cerr << "WARNING: Failed to load template file: " << template_file << "\n";
    }
}

bool OllamaAPI::requestProgram(
    const std::string& features_json,
    std::string& program_text,
    std::string& raw_response
) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string url = "http://localhost:11434/api/generate";
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Build the prompt
    std::string prompt =
        "You are a CAD/CAM G-code generator. Generate G-code for the provided features.\n"
        "Return ONLY JSON with: {\"program_text\": \"...gcode...\"}\n\n"
        "FEATURE INPUT JSON:\n" + features_json + "\n";

    // Escape JSON for Ollama
    auto escapeJsonString = [](const std::string& input) -> std::string {
        std::string output;
        for (char c : input) {
            switch (c) {
                case '\"': output += "\\\""; break;
                case '\\': output += "\\\\"; break;
                case '\n': output += "\\n"; break;
                case '\t': output += "\\t"; break;
                default: output += c;
            }
        }
        return output;
    };

    std::string escaped = escapeJsonString(prompt);

    // Build POST body
    std::string payload =
        "{"
        "\"model\": \"" + model_ + "\","
        "\"prompt\": \"" + escaped + "\""
        "}";

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
    
    std::cout << "Sending request to Ollama (120s timeout)...\n";

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Curl error: " << curl_easy_strerror(res) << "\n";
        return false;
    }

    raw_response = response;

    // Look for "ERROR - INVALID INPUT"
    if (response.find("ERROR - INVALID INPUT") != std::string::npos) {
        program_text = "ERROR - INVALID INPUT";
        return true;
    }

    // Extract all "response" parts from streamed JSON and concatenate
    std::string full_response;
    size_t pos = 0;
    auto resp_key = "\"response\":\"";
    
    while ((pos = response.find(resp_key, pos)) != std::string::npos) {
        pos += std::strlen(resp_key);
        size_t end_quote = response.find('"', pos);
        if (end_quote != std::string::npos) {
            full_response += response.substr(pos, end_quote - pos);
            pos = end_quote;
        } else {
            break;
        }
    }
    
    if (!full_response.empty()) {
        program_text = full_response;
        return true;
    }

    program_text = response; // fallback: return raw
    return true;
}

