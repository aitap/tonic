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

find_package(ALSA)

include_directories(${IUP_INC} ${PORTMIDI_INC})
link_directories(${IUP_LIB} ${PORTMIDI_LIB} ${ALSA_LIBRARY})
add_executable(tonic main.c game.c keys.c)
target_link_libraries(tonic iup portmidi porttime)
