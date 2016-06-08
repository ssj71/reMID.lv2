
cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig)
pkg_check_modules(CAIRO cairo REQUIRED)

pkg_check_modules(X11 x11)
pkg_check_modules(USER32 user32)
pkg_check_modules(COCOA Cocoa)

include_directories( ../avtk pugl ${CAIRO_INCLUDE_DIRS} )
link_directories( ${CAIRO_LIBRARY_DIRS} )
add_definitions( "-DPUGL_HAVE_CAIRO" )

if(X11_FOUND)
    list(APPEND PUGL_SRC pugl/pugl_x11.c )
#    include_directories( ${X11_INCLUDE_DIRS} )
#    link_directories( ${X11_LIBRARY_DIRS} )
elseif(USER32_FOUND)
    pkg_check_modules(OPENGL32 opengl32 REQUIRED )
    pkg_check_modules(GLU32 glu32 REQUIRED )
    pkg_check_modules(GDI32 gdi32 REQUIRED )
    list(APPEND PUGL_SRC pugl/pugl_win.cpp ) 
    include_directories( ${USER32_INCLUDE_DIRS} ${OPENGL32_INCLUDE_DIRS} ${GLU32_INCLUDE_DIRS} ${GDI32_INCLUDE_DIRS} )
    link_directories( ${USER32_LIBRARY_DIRS} ${OPENGL32_LIBRARY_DIRS} ${GLU32_LIBRARY_DIRS} ${GDI32_LIBRARY_DIRS} )
elseif(COCOA_FOUND)
    pkg_check_modules(OPENGL OpenGL REQUIRED )
    list(APPEND PUGL_SRC pugl/pugl_osx.m )  
    include_directories( ${COCOA32_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} )
    link_directories( ${COCOA32_LIBRARY_DIRS} ${OPENGL_LIBRARY_DIRS} )
else()
    message(SEND_ERROR "NO APPROPRIATE UI LIBRARY FOUND (x11, cocoa, nor user32)")

endif()

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
    ${PUGL_SRC}
)

target_link_libraries( avtk LINK_PUBLIC ${CAIRO_LIBRARY} )
if(X11_FOUND)
    target_link_libraries( avtk ${X11_LIBRARY} )
elseif(USER32_FOUND)
elseif(COCOA_FOUND)
endif()
