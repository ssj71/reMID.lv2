#CMake file for osc2midi

cmake_minimum_required(VERSION 2.8)

project (reMID)

set(CMAKE_C_FLAGS "-Wall -g -DPREFIX='\"${CMAKE_INSTALL_PREFIX}\"'")

# check for our various libraries
find_package(PkgConfig)
find_package(Jack)
find_package(GTK2 2.10 COMPONENTS gtk glade)
pkg_check_modules(Alsa alsa)
find_path(RESID_INCLUDE_DIRS resid/sid.h)
find_library(RESID_LIBRARIES resid PATH_SUFFIXES resid)

include_directories (${JACK_INCLUDE_DIRS} ${RESID_INCLUDE_DIRS})
link_directories (${ALSA_LIBRARY_DIRS} ${JACK_LIBRARY_DIRS})

if(ALSA_FOUND)
 
include_directories (${ALSA_INCLUDE_DIRS})
link_directories (${ALSA_LIBRARY_DIRS})
endif()
if(GTK2_FOUND)
   include_directories(${GTK2_INCLUDE_DIRS})
   #add_executable(mygui mygui.cc)
   #target_link_libraries(mygui ${GTK2_LIBRARIES})
endif()

# config libraries

add_executable(reMID
    alsa_midi.c
    jack_audio.c
    jack_midi.c
    midi.c
    prefs.c
    reMID.c
    sid_chips.cc
)


target_link_libraries(reMID ${LO_LIBRARIES} ${JACK_LIBRARIES} m)

# config install
install(TARGETS reMID
  DESTINATION bin
)

