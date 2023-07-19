cmake_minimum_required(VERSION 2.8)
project(joinImages)
find_package( OpenCV REQUIRED PATHS /home/user/opencv/build NO_DEFAULT_PATH)
add_executable( joinImages src/joinImages.cpp)
target_link_libraries( joinImages ${OpenCV_LIBS})

