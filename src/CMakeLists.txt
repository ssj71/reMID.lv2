#CMake file for osc2midi

cmake_minimum_required(VERSION 2.8)

project (reMID)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -std=c99 -DPREFIX='\"${CMAKE_INSTALL_PREFIX}\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

# check for our various libraries
find_package(PkgConfig)
find_package(Jack)
#find_package(GTK2 2.10 COMPONENTS gtk glade)
pkg_check_modules(ALSA alsa)
pkg_check_modules(GLIB glib-2.0)
pkg_check_modules(LV2 lv2) 
#find_path(RESID_INCLUDE_DIRS resid/sid.h)
#find_library(RESID_LIBRARIES resid PATH_SUFFIXES resid)

#message(${RESID_INCLUDE_DIRS})
#message("here's the libs")
#message(${RESID_LIBRARIES})
#message("here's the glibs")

#message(${GLIB_LIBRARIES})
#message(${JACK_LIBRARIES})

include_directories (${JACK_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${LV2_INCLUDE_DIRS})
link_directories ( ${JACK_LIBRARY_DIRS} ${GLIB_LIBRARY_DIRS} ${LV2_LIBRARY_DIRS})

#by default, we're going to use jack midi
#if(ALSA_FOUND) 
#	include_directories (${ALSA_INCLUDE_DIRS})
#	link_directories (${ALSA_LIBRARY_DIRS})
#	#add_definitions(-DALSA_MIDI)
#endif()

#I decided to just define this in midi.c
#add_definitions(-DJACK_MIDI)

#I can't get the UI to work (glade version too old)
#if(GTK2_FOUND)
#   include_directories(${GTK2_INCLUDE_DIRS})
#   #add_executable(mygui mygui.cc)
#   #target_link_libraries(mygui ${GTK2_LIBRARIES})
#endif()

# config libraries

add_executable(reMID
    #alsa_midi.c
    reMID.c
    audio.c
    jack_midi.c
    midi.c
    prefs.c
    sid_chips.cc
)

#add_definitions(-DLV2)
add_library(remid.lv2 SHARED
    sid_chips.cc
    reMID_lv2.c
    audio.c
    lv2_midi.c
    midi.c
    prefs.c
)

#set_target_properties(remid.lv2 PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
#SET_TARGET_PROPERTIES(remid.lv2 PROPERTIES COMPILE_FLAGS "-fPIC")

#NOTE: exactly ONE of the following should be uncommented, choose alsa or jack
SET_TARGET_PROPERTIES(reMID PROPERTIES COMPILE_FLAGS -DJACK_MIDI)
#SET_TARGET_PROPERTIES(reMID PROPERTIES COMPILE_FLAGS -DALSA_MIDI)
    
SET_TARGET_PROPERTIES(remid.lv2 PROPERTIES COMPILE_FLAGS -DLV2)
set_target_properties(remid.lv2 PROPERTIES PREFIX "")
#set_target_properties(remid.lv2 PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)

target_link_libraries(reMID resid ${JACK_LIBRARIES} ${GLIB_LIBARIES} glib-2.0 m resid)
target_link_libraries(remid.lv2 resid ${LV2_LIBRARIES} ${GLIB_LIBARIES} glib-2.0 m resid)

# config install
install(TARGETS reMID
  DESTINATION bin
)

install(TARGETS remid.lv2
  LIBRARY
  DESTINATION lib/lv2/remid.lv2
)
install (FILES manifest.ttl remid.ttl
  DESTINATION lib/lv2/remid.lv2
)
