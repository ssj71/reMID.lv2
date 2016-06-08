cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

add_library(avtk STATIC 
box.cxx
button.cxx
common.cxx
dial.cxx
dialog.cxx
envelope.cxx
eventeditor.cxx
filebrowser.cxx
group.cxx
helpers.cxx
image.cxx
list.cxx
listitem.cxx
midi.cxx
number.cxx
scroll.cxx
slider.cxx
spectrum.cxx
tester.cxx
text.cxx
theme.cxx
ui.cxx
#utils.cxx ##this file has deps on libsndfile, that could be factored out though
waveform.cxx
widget.cxx
)

add_executable(test_ui
    
)
