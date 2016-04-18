#include <portmidi.h>
#include <porttime.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iup.h>
#include "tonic.h"

/* globals, ewwwwww */
PmStream* midi = NULL;
PmDeviceID* outs = NULL;
size_t num_outs = 0;
Ihandle* device_list = NULL; /* select MIDI output */
Ihandle* program_number = NULL; /* enter instrument number */
Ihandle *key_text, *chord_text; /* show current key and whether the guess was correct */

const int32_t latency = 16; /* I need timestamps, and 16ms seems reasonable */

PmError show_if_pm_error(PmError code) {
	char os_error[1024];
	if (code == pmNoError) return code;
	if (code == pmHostError)
		Pm_GetHostErrorText(os_error, 1024);
	IupMessage("PortMidi fatal error", code == pmHostError ? os_error : Pm_GetErrorText(code));
	return code;
}

int open_audio_callback(Ihandle* button) {
	PmError err;

	if (midi) 
    	if (show_if_pm_error(Pm_Close(midi)) != pmNoError)
			return IUP_DEFAULT;

	midi=NULL;

	int out_number = IupGetInt(device_list, "VALUE") - 1;
	/* assure compiler I know what I'm doing */
	assert(out_number < (int)num_outs);
	assert(out_number >= 0);
	
    show_if_pm_error(Pm_OpenOutput(
		&midi /* struct to work with */,
		outs[out_number] /* number of MIDI output */,
		NULL /* optional driver-specific wtf */,
		0 /* apparently I don't need to buffer */,
		NULL /* default timing function */,
		NULL /* no arguments to timing function, either */,
		latency
	));
	/* FIXME: only at this point it's safe to continue, otherwise app should quit or something */
	return IUP_DEFAULT;
}

int set_program_callback(Ihandle* button) {
	show_if_pm_error(Pm_WriteShort(midi,0,Pm_Message(0xC0, IupGetInt(program_number,"VALUE"), 0)));
}	

int main(int argc, char** argv) {
	IupOpen(&argc, &argv);

	{
		PmError err;
		if (show_if_pm_error(Pm_Initialize()) != pmNoError) {
			return (int)err;
		}
	}

	PmDeviceID out = Pm_GetDefaultOutputDeviceID();
	if (out == pmNoDevice) {
		IupMessage("No MIDI devices", "Apparently, there are no MIDI output devices on your machine. Perhaps you need to run a software synsthesizer?");
		return 1;
	}
	/* at this point we probably have at least one output device */

	device_list = IupList(NULL); /* ask the user which output device they want */

	{
		int num_devices = Pm_CountDevices();
		outs = calloc(num_devices, sizeof(PmDeviceID));
	
		for (PmDeviceID i = 0; i < num_devices; i++) {
			if (Pm_GetDeviceInfo(i)->output) {
				outs[num_outs] = i;
				num_outs++;
			}
		}
		/* at this point valid device IDs are in range 0..num_outs-1 */

		for (size_t j = 0; j < num_outs; j++) {
			size_t length = snprintf(NULL, 0, "%d", j)+1;
			char* att_name = malloc(length);
			assert(att_name); // I don't think there's a way to continue working there
			snprintf(att_name, length, "%d", j);
			IupSetAttribute(device_list, att_name, Pm_GetDeviceInfo(outs[j])->name);
			free(att_name);
		}
	}
	IupSetAttribute(device_list, "DROPDOWN", "YES");
	IupSetInt(device_list, "VALUE", out+1);
	open_audio_callback(NULL); // everything is set up => try to open out device

	program_number = IupSetAttributes(IupText(NULL),"VALUE=0,SPIN=YES,SPINMIN=0,SPINMAX=255,SPININC=1");
	IupSetAttribute(program_number, "MASK", IUP_MASK_INT);

	key_text = IupLabel("<key>");
	chord_text = IupLabel("<chord>");

#define IupHorizExpand(x) IupSetAttributes((x),"EXPAND=HORIZONTAL")
#define IupAlignCenter(x) IupSetAttributes((x),"ALIGNMENT=ACENTER:ACENTER")
	IupShow(IupSetAttributes(
		IupDialog(IupVbox(
			IupHbox(IupHorizExpand(device_list),IupSetCallbacks(IupButton("Open",NULL),"ACTION",(Icallback)open_audio_callback,NULL),NULL),
			IupHbox(
				IupHorizExpand(program_number),
				IupSetCallbacks(IupButton("Set program",NULL),"ACTION",(Icallback)set_program_callback,NULL),
			NULL),
			IupHorizExpand(IupAlignCenter(key_text)), IupHorizExpand(IupAlignCenter(chord_text)),
			IupHorizExpand(IupAlignCenter(IupLabel("1..7 - guess\nspace - play again\nenter - new key"))),
		NULL)),
	"TITLE=\"Tonic\",RESIZE=NO"));
	IupMainLoop();
	IupClose();

#if 0
	/* construct & write on the fly like this */
    /* short note-on */
    Pm_WriteShort(midi, Pt_Time(),
                  Pm_Message(0x90, 60, 100));
    /* short note-off */
    Pm_WriteShort(midi, Pt_Time(),
                  Pm_Message(0x90, 60, 0));
#endif

#if 0
	/* see test.c for a working example without 100%CPU busy-loop */
    /* output several note on/offs to test timing. 
       Should be 1s between notes */
    /* arpeggio chord (because latency is on) */
    timestamp = Pt_Time();
    for (int i = 0; i < chord_size; i++) {
        buffer[i].timestamp = timestamp + diff * i;
        buffer[i].message = Pm_Message(0x90, chord[i], 100);
    }
    Pm_Write(midi, buffer, chord_size);

    off_time = timestamp + diff + chord_size * diff; 
    while (Pt_Time() < off_time) 
		/* busy wait */; /* ewwwww */

    for (int i = 0; i < chord_size; i++) {
        buffer[i].timestamp = timestamp + diff * i;
        buffer[i].message = Pm_Message(0x90, chord[i], 0);
    }
    Pm_Write(midi, buffer, chord_size);    
#endif

    Pm_Close(midi);
    Pm_Terminate();

	IupClose();

	free(outs);
	return 0;
}

