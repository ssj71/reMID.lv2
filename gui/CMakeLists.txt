cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

#TO USE AVTK YOU JUST NEED TO DO 4 COMMANDS
add_subdirectory(avtk)
include_directories( ${AVTK_INCLUDE_DIRS} )
#link_directories( ${AVTK_LIBRARY_DIRS} )

#debug stuff
#message( "include"  ${AVTK_INCLUDE_DIRS} )
#message(  "link" ${AVTK_LIBRARY_DIRS} )

# YOUR PROJECT SPECIFIC STUFF HERE
add_executable(test_ui
    test_ui.cxx
    main.cxx
)

#THE FINAL COMMAND FOR AVTK
target_link_libraries(test_ui avtk ${AVTK_LDFLAGS} )
