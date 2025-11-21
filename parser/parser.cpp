#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " input.nc\n";
        return 1;
    }

    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        std::cerr << "Cannot open input file\n";
        return 1;
    }

    std::string outputName = "parsed_" + std::string(argv[1]);
    std::ofstream output(outputName);
    std::string line;

    while (std::getline(input, line)) {
        // Look for the G-code pattern: \G21 G90\\n...
        size_t gcode_start = line.find("\\G");
        if (gcode_start != std::string::npos) {
            // Find the end marker \}
            size_t gcode_end = line.find("\\}", gcode_start);
            if (gcode_end != std::string::npos) {
                // Extract the G-code content
                std::string gcode = line.substr(gcode_start + 1, gcode_end - gcode_start - 1);
                
                // Convert \\n to actual newlines
                size_t pos = 0;
                while ((pos = gcode.find("\\\\n", pos)) != std::string::npos) {
                    gcode.replace(pos, 3, "\n");
                    pos += 1;
                }
                
                // Write to output, splitting on newlines
                std::string temp;
                for (char c : gcode) {
                    if (c == '\n') {
                        if (!temp.empty()) {
                            output << temp << "\n";
                            temp.clear();
                        }
                    } else {
                        temp += c;
                    }
                }
                
                if (!temp.empty()) {
                    output << temp << "\n";
                }
            }
        }
    }

    std::cout << "Done. Output written to " << outputName << "\n";
    return 0;
}