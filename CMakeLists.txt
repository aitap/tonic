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

if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
	add_compile_options(-std=c99)
endif()

set (TONIC_SOURCE main.c game.c keys.c logo/tonic_64x64_rgba.c)
if(WIN32)
	# support modern UI and high DPI
	set(TONIC_SOURCE ${TONIC_SOURCE} iup.rc)
endif()

add_executable(tonic ${TONIC_SOURCE})
if(WIN32)
	# apparently, on Win32 portmidi consists of one DLL
	set(PORTLIBRARIES portmidi)
	# tonic.exe is a GUI app
	set_target_properties(tonic PROPERTIES LINK_FLAGS -mwindows)
else()
	set(PORTLIBRARIES portmidi porttime)
endif()

target_link_libraries(tonic iup ${PORTLIBRARIES})
