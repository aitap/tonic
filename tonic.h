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

extern PmStream* midi;
extern PmDeviceID* outs;
extern size_t num_outs;
extern Ihandle* device_list;
extern Ihandle* program_number;
extern Ihandle *key_text, *chord_text;

PmTimestamp my_timer(void*);

int keypress_callback(Ihandle*, int);
void change_key(void);

extern int current_key;
extern int current_note;
extern int current_minor;

typedef struct {
	const char* major_name;
	const char* minor_name;
	const int major_tonic;
	const int minor_tonic;
	const int signature[7];
} tone_struct;

extern tone_struct keys[];
extern size_t keys_size;
extern char* steps[];
extern size_t steps_size;
extern int major_semitones[];
extern int minor_semitones[];
