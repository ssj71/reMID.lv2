
cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig)
pkg_check_modules(CAIRO cairo REQUIRED)
pkg_check_modules(SND sndfile )

list(APPEND AVTK_INCLUDE_DIRS ${CAIRO_INCLUDE_DIRS} avtk ) #note this extra reference to avtk is for the parent dir, not here
list(APPEND AVTK_LDFLAGS ${CAIRO_LDFLAGS} )
#list(APPEND AVTK_LIBRARY_DIRS ${CAIRO_LIBRARY_DIRS} )

list(APPEND AVTK_SND_SRC "")
if(SND_FOUND)
    add_definitions( "-DAVTK_SNDFILE" )
    list(APPEND SND_SRC snd_utils.cxx)
    list(APPEND AVTK_INCLUDE_DIRS ${SND_INCLUDE_DIRS} ) 
    list(APPEND AVTK_LDFLAGS ${SND_LDFLAGS} )
endif()

#platform specific libs
pkg_check_modules(X11 x11)
pkg_check_modules(USER32 user32)
pkg_check_modules(COCOA Cocoa)


add_definitions( "-DPUGL_HAVE_CAIRO" )

if(X11_FOUND)
    list(APPEND PUGL_SRC pugl/pugl_x11.c )
    list(APPEND AVTK_INCLUDE_DIRS ${X11_INCLUDE_DIRS} )
    list(APPEND AVTK_LDFLAGS ${X11_LDFLAGS} )
elseif(USER32_FOUND)
    pkg_check_modules(OPENGL32 opengl32 REQUIRED )
    pkg_check_modules(GLU32 glu32 REQUIRED )
    pkg_check_modules(GDI32 gdi32 REQUIRED )
    list(APPEND PUGL_SRC pugl/pugl_win.cpp ) 
    list(APPEND AVTK_INCLUDE_DIRS ${USER32_INCLUDE_DIRS} ${OPENGL32_INCLUDE_DIRS} ${GLU32_INCLUDE_DIRS} ${GDI32_INCLUDE_DIRS} )
    list(APPEND AVTK_LDFLAGS ${USER32_LDFLAGS} ${OPENGL32_LDFLAGS} ${GLU32_LDFLAGS} ${GDI32_LDFLAGS} )
elseif(COCOA_FOUND)
    pkg_check_modules(OPENGL OpenGL REQUIRED )
    list(APPEND PUGL_SRC pugl/pugl_osx.m )  
    list(APPEND AVTK_INCLUDE_DIRS ${COCOA32_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} )
    list(APPEND AVTK_LDFLAGS ${COCOA32_LDFLAGS} ${OPENGL_LDFLAGS} )
else()
    message(SEND_ERROR "NO APPROPRIATE UI LIBRARY FOUND (x11, cocoa, nor user32)")

endif()

set(AVTK_INCLUDE_DIRS ${AVTK_INCLUDE_DIRS} PARENT_SCOPE)
set(AVTK_LDFLAGS ${AVTK_LDFLAGS} PARENT_SCOPE)
#set(AVTK_LIBRARY_DIRS ${AVTK_LIBRARY_DIRS} PARENT_SCOPE)

include_directories( ${AVTK_INCLUDE_DIRS} ../avtk pugl )
#link_directories( ${AVTK_LIBRARY_DIRS} )


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
    utils.cxx
    ${AVTK_SND_SRC}
    waveform.cxx
    widget.cxx
    ${PUGL_SRC}
)
