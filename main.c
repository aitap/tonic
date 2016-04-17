#include <portmidi.h>
#include <porttime.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iup.h>

const int32_t latency = 16; /* I need timestamps, and 16ms seems reasonable */
const int program = 0; /* piano; FIXME */

void show_pm_error(const char* title, PmError code) {
	char os_error[1024];
	if (code == pmHostError)
		Pm_GetHostErrorText(os_error, 1024);
	IupMessage(title, code == pmHostError ? os_error : Pm_GetErrorText(code));
}

int main(int argc, char** argv) {
	IupOpen(&argc, &argv);

	PmStream* midi;
	PmError err;

	if ((err = Pm_Initialize()) != pmNoError) {
		show_pm_error("PortMidi fatal error",err);
		return (int)err;
	}

	PmDeviceID out = Pm_GetDefaultOutputDeviceID();
	if (out == pmNoDevice) {
		IupMessage("No MIDI devices", "Apparently, there are no MIDI output devices on your machine. Perhaps you need to run a software synsthesizer?");
		return 1;
	}

	int num_devices = Pm_CountDevices();
	/* TODO: logic to ask user for an output device, IF there are >1 */
	out = 2; /* FIXME */

    /* open output device -- since PortMidi avoids opening a timer
       when latency is zero, we will pass in a NULL timer pointer
       for that case. If PortMidi tries to access the time_proc,
       we will crash, so this test will tell us something. */
    if ((err = Pm_OpenOutput(
		&midi /* struct to work with */,
		out /* number of MIDI output */,
		NULL /* optional driver-specific wtf */,
		0 /* apparently I don't need to buffer */,
		NULL /* default timing function */,
		NULL /* no arguments to timing function, either */,
		latency
	)) != pmNoError) {
		show_pm_error("PortMidi fatal error", err);
		return err;
	}

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
	return 0;
}

