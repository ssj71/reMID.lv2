cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

#TO USE AVTK YOU JUST NEED TO DO 4 COMMANDS
add_subdirectory(avtk)
include_directories( avtk )
#link_directories( avtk )
#include_directories( avtk ${CAIRO_LIBRARY_DIRS} )
#link_directories( ${CAIRO_LIBRARY_DIRS} )
#message( ${CAIRO_LIBRARY_DIRS} )

# YOUR PROJECT SPECIFIC STUFF HERE
add_executable(test_ui
    test_ui.cxx
    main.cxx
)

#THE FINAL COMMAND FOR AVTK
target_link_libraries(test_ui avtk)
