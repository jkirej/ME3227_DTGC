cmake_minimum_required(VERSION 3.16)
project(cad_to_gcode_cpp_full LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TESSERACT tesseract)
find_package(CURL REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} ${TESSERACT_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/third_party)
link_directories(${TESSERACT_LIBRARY_DIRS})

file(GLOB SRC src/*.cpp)
add_executable(cad_to_gcode ${SRC})
target_include_directories(cad_to_gcode PRIVATE include)
target_link_libraries(cad_to_gcode PRIVATE ${OpenCV_LIBS} ${TESSERACT_LIBRARIES} ${CURL_LIBRARIES} lept)

