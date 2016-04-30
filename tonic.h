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

// globals
//  midi out-related variables
extern PmStream* midi; // current output
extern PmDeviceID* outs; // array of output indices
extern size_t num_outs; // size of array of output indices
//  interface handles
extern Ihandle* device_list; // list of midi outs
extern Ihandle* program_number; // instrument number
extern Ihandle *key_text, *chord_text; // current key and previous answer
//  game variables
extern int current_key;
extern int current_note;
extern bool current_minor;

// exported functions
//  main.c
PmTimestamp my_timer(void*); // timer to plan chords in sound_chord
PmError show_if_pm_error(PmError); // signal if an error occurs
//  game.c
int keypress_callback(Ihandle*, int); // handle key presses
void change_key(void); // generate a new key

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

/* interface defines */
#define tonic_single_note_checkbox "tonic_single_note_checkbox"
