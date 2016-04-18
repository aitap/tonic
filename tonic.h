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

int keypress_callback(Ihandle* dialog, int pressed);

extern int current_key;
extern int current_note;
