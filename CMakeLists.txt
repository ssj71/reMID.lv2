cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/")

add_subdirectory(lib/) 
add_subdirectory(src/) 
install (DIRECTORY instruments/
    DESTINATION lib/lv2/remid.lv2
)
