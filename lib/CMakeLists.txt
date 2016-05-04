#CMake file for osc2midi

cmake_minimum_required(VERSION 2.8)

project (reMID)

# config library 
add_definitions(-DVERSION="0.16")
add_library(resid STATIC
    sid.cc voice.cc wave.cc envelope.cc filter.cc extfilt.cc pot.cc version.cc wave6581_PST.cc wave6581_PS_.cc wave6581_P_T.cc wave6581__ST.cc wave8580_PST.cc wave8580_PS_.cc wave8580_P_T.cc wave8580__ST.cc
)

set_target_properties(resid PROPERTIES POSITION_INDEPENDENT_CODE TRUE)


# config install
#install(TARGETS reMID
#  LIBRARY
#  DESTINATION lib
#)
