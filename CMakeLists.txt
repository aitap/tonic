cmake_minimum_required(VERSION 3.0)
project(Tonic C)

find_library(IUP_LIB
	NAMES iup libiup
)

find_path(IUP_INC
	NAMES iup.h
)

find_path(PORTMIDI_INC
	NAMES portmidi.h
)

find_path(PORTMIDI_LIB
	NAMES portmidi libportmidi
)

include_directories(${IUP_INC} ${PORTMIDI_INC})
link_directories(${IUP_LIB} ${PORTMIDI_LIB})

if(WIN32)
	# apparently, on Win32 portmidi consists of one DLL
	set(PORTLIBRARIES portmidi)
	# tonic.exe is a GUI app
	add_compile_options(-mwindows)
else()
	set(PORTLIBRARIES portmidi porttime)
endif()
add_executable(tonic main.c game.c keys.c)
target_link_libraries(tonic iup ${PORTLIBRARIES})
