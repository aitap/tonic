#include <portmidi.h>
#include <porttime.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iup.h>

const int32_t latency = 16;
const int out = 2; /* FIXME */
const int program = 0; /* piano; FIXME */
const int diff = 100;

int main(int argc, char** argv) {
	PmStream* midi;
    int32_t off_time;
    int chord[] = { 60, 67, 76, 83, 90 };
    #define chord_size 5 
    PmEvent buffer[chord_size];
    PmTimestamp timestamp;

    /* It is recommended to start timer before PortMidi */
    Pt_Start(
		1 /* resolution, ms */,
		NULL /* no callback */,
		NULL /* no callback parameter, either */
	);
	
	assert(!Pm_Initialize()); /* pmNoError = 0 */

    /* open output device -- since PortMidi avoids opening a timer
       when latency is zero, we will pass in a NULL timer pointer
       for that case. If PortMidi tries to access the time_proc,
       we will crash, so this test will tell us something. */
    Pm_OpenOutput(&midi, 
                  out /* FIXME: hardcoded number */, 
                  NULL /* optional driver-specific wtf */,
                  0 /* apparently I don't need to buffer */ , 
                  ((int32_t (*)(void *)) Pt_Time) /* timing function */,
                  NULL, 
                  latency);
    printf("Midi Output opened with %ld ms latency.\n", (long) latency);

    /* if we were writing midi for immediate output, we could always use
       timestamps of zero, but since we may be writing with latency, we
       will explicitly set the timestamp to "now" by getting the time.
       The source of timestamps should always correspond to the TIME_PROC
       and TIME_INFO parameters used in Pm_OpenOutput(). */
    buffer[0].timestamp = Pt_Time();

    /* Send a program change to increase the chances we will hear notes */
    /* Program 0 is usually a piano, but you can change it here: */
    buffer[0].message = Pm_Message(0xC0, program, 0);
    Pm_Write(midi, buffer, 1);

    /* note-on */
    buffer[0].timestamp = Pt_Time();
    buffer[0].message = Pm_Message(0x90, 60, 100);
    Pm_Write(midi, buffer, 1);

    /* note-off */
    buffer[0].timestamp = Pt_Time();
    buffer[0].message = Pm_Message(0x90, 60, 0);
    Pm_Write(midi, buffer, 1);

    /* short note-on */
    Pm_WriteShort(midi, Pt_Time(),
                  Pm_Message(0x90, 60, 100));
    /* short note-off */
    Pm_WriteShort(midi, Pt_Time(),
                  Pm_Message(0x90, 60, 0));

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

    /* exterminate */
    Pm_Close(midi);
    Pm_Terminate();
	return 0;
}

