cmake_minimum_required(VERSION 2.8)
project(lerImagem)
find_package( OpenCV REQUIRED PATHS /home/user/build NO_DEFAULT_PATH)
add_executable( joinImages joinImages.cpp )
target_link_libraries( joinImages ${OpenCV_LIBS})

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
