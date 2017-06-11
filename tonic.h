#pragma once
#include <portmidi.h>
#include <porttime.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iup.h>
#include <iupkey.h>
#include <time.h>
#include <stdbool.h>

struct game {
// MIDI handles
	PmStream* midi; // current output
	PmDeviceID* outs; // array of output devices
//  game variables
	int key;
	int note;
	bool minor;
	bool single;
};

// exported functions
//  main.c
PmTimestamp my_timer(void*); // timer to plan chords in sound_chord
PmError show_if_pm_error(PmError); // signal if an error occurs
//  game.c
void change_key(struct game*); // generate a new key
void check_guess(struct game *, int); // check user's guess
void sound_chord(struct game *, int, bool); // sound chord/note from current key
void change_key(struct game*); // change current key

// tables of constants
typedef struct {
	const char* major_name;
	const char* minor_name;
	const int major_tonic;
	const int minor_tonic;
} tone_struct;

extern tone_struct keys[]; // circle of fifths, tabulated
extern size_t keys_size; // size of ^^^
extern char* steps[]; // names of steps of a key
extern size_t steps_size; // size of ^^^
extern int major_semitones[]; // offsets in semitones between a tonic and a note in a major key
extern int minor_semitones[]; // likewise, but minor
