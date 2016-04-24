#CMake file for osc2midi

cmake_minimum_required(VERSION 2.8)

project (reMID)

set(CMAKE_C_FLAGS "-Wall -g -DPREFIX='\"${CMAKE_INSTALL_PREFIX}\"'")

# check for our various libraries
find_package(PkgConfig)
find_package(Jack)
#find_package(GTK2 2.10 COMPONENTS gtk glade)
pkg_check_modules(ALSA alsa)
pkg_check_modules(GLIB glib-2.0)
pkg_check_modules(LV2 lv2) 
find_path(RESID_INCLUDE_DIRS resid/sid.h)
find_library(RESID_LIBRARIES resid PATH_SUFFIXES resid)

message(${RESID_INCLUDE_DIRS})
message("here's the libs")
message(${RESID_LIBRARIES})
message("here's the glibs")

message(${GLIB_LIBRARIES})
message(${JACK_LIBRARIES})

include_directories (${JACK_INCLUDE_DIRS} ${RESID_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS} ${LV2_INCLUDE_DIRS})
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

#add_definitions(-DLV2_PLUGIN)
add_library(remid.lv2 SHARED
    reMID_lv2.c
    audio.c
    lv2_midi.c
    midi.c
    prefs.c
    sid_chips.cc
)
    

target_link_libraries(reMID ${JACK_LIBRARIES} ${GLIB_LIBARIES} ${RESID_LIBARIES} glib-2.0 m resid)

# config install
install(TARGETS reMID
  DESTINATION bin
)

