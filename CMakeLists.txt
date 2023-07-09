cmake_minimum_required(VERSION 2.8)
project(joinImages)
find_package( OpenCV REQUIRED PATHS /home/henrique/opencv/build NO_DEFAULT_PATH)
add_executable( joinImages joinImages.cpp)
target_link_libraries( joinImages ${OpenCV_LIBS})

