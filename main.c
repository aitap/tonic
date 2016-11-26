#include "tonic.h"
/* the icon */
extern unsigned char tonic_64x64_rgba[];

/* globals, ewwwwww */
PmStream* midi = NULL;
PmDeviceID* outs = NULL;
Ihandle* device_list = NULL; /* select MIDI output */
Ihandle* program_number = NULL; /* enter instrument number */
Ihandle *key_text, *chord_text; /* show current key and whether the guess was correct */
Ihandle* single_note_checkbox = NULL; /* play single notes, not triads */

const int32_t latency = 16; /* I need timestamps, and 16ms seems reasonable */

PmError show_if_pm_error(PmError code) {
	char os_error[1024];
	if (code == pmNoError) return code;
	if (code == pmHostError)
		Pm_GetHostErrorText(os_error, 1024);
	IupMessage("PortMidi error", code == pmHostError ? os_error : Pm_GetErrorText(code));
	return code;
}

PmTimestamp my_timer (void* wtf) {
	(void)wtf; /* Pt_Time doesn't need any arguments, but PmTimeProc does */
	return (PtTimestamp)Pt_Time();
}

int open_audio_callback(Ihandle* button) {
	if (midi) 
    	if (show_if_pm_error(Pm_Close(midi)) != pmNoError)
			return IUP_DEFAULT;

	midi=NULL;

	int out_number = IupGetInt(device_list, "VALUE") - 1;
	
	show_if_pm_error(Pm_OpenOutput(
		&midi /* struct to work with */,
		outs[out_number] /* number of MIDI output */,
		NULL /* optional driver-specific wtf */,
		0 /* apparently I don't need to buffer */,
		(PmTimeProcPtr)my_timer /* adapt porttime to portmidi's function pointer requirements */,
		NULL /* no arguments to timing function, either */,
		latency
	));

	/* PortMidi seems to return error codes if given a NULL, so no abort() here */
	return IUP_DEFAULT;
}

int set_program_callback(Ihandle* button) {
	show_if_pm_error(Pm_WriteShort(midi,0,Pm_Message(0xC0, IupGetInt(program_number,"VALUE"), 0)));
	return IUP_DEFAULT;
}	

int main(int argc, char** argv) {
	IupOpen(&argc, &argv);

	{
		PtError err;
		if ((err = Pt_Start(1 /* resolution, ms */, NULL /* no callback */, NULL)) != ptNoError) {
			IupMessagef("PortTime error", "Pt_Start returned error code %d", err);
			return (int)err;
		}
	}
	/* note: these are different kinds of errors */
	{
		PmError err;
		if ((err = show_if_pm_error(Pm_Initialize())) != pmNoError) {
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

	size_t num_outs = 0;
	{
		int num_devices = Pm_CountDevices();
		outs = calloc(num_devices, sizeof(PmDeviceID));
		assert(outs);
	
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
			snprintf(att_name, length, "%d", j+1);
			IupSetAttribute(device_list, att_name, Pm_GetDeviceInfo(outs[j])->name);
			free(att_name);
		}
	}
	IupSetAttribute(device_list, "DROPDOWN", "YES");
	IupSetInt(device_list, "VALUE", out+1);
	open_audio_callback(NULL); // everything is set up => try to open out device

	program_number = IupSetAttributes(IupText(NULL),"VALUE=0,SPIN=YES,SPINMIN=0,SPINMAX=255,SPININC=1");
	IupSetAttribute(program_number, "MASK", IUP_MASK_INT);

	key_text = IupLabel("___-____");
	IupSetInt(key_text,"FONTSIZE",IupGetInt(key_text,"FONTSIZE")*2);
	chord_text = IupLabel("__");
	IupSetInt(chord_text,"FONTSIZE",IupGetInt(chord_text,"FONTSIZE")*4);

	srand((unsigned int)time(NULL));
	change_key();

	single_note_checkbox = IupSetAttributes(IupToggle("Single notes",NULL),"VALUE=ON");

	IupSetHandle("Tonic_icon",IupImageRGBA(64,64,tonic_64x64_rgba));

#define IupHorizExpand(x) IupSetAttributes((x),"EXPAND=HORIZONTAL")
#define IupAlignCenter(x) IupSetAttributes((x),"ALIGNMENT=ACENTER:ACENTER")
	IupShow(IupSetAttributes(
		IupSetCallbacks(IupDialog(IupVbox(
			IupHbox(IupHorizExpand(device_list),IupSetCallbacks(IupButton("Open",NULL),"ACTION",(Icallback)open_audio_callback,NULL),NULL),
			IupHbox(
				IupHorizExpand(program_number),
				IupSetCallbacks(IupButton("Set program",NULL),"ACTION",(Icallback)set_program_callback,NULL),
			NULL),
			single_note_checkbox,
			IupHorizExpand(IupAlignCenter(key_text)), IupHorizExpand(IupAlignCenter(chord_text)),
			IupHorizExpand(IupAlignCenter(IupLabel("guess: ctrl+1..7\nplay chord again: =\nplay tonic again: t\nnew key: -"))),
		NULL)),"K_ANY",(Icallback)keypress_callback,NULL),
	"TITLE=\"Tonic\",RESIZE=NO,ICON=\"Tonic_icon\""));

	IupMainLoop();

	Pm_Close(midi);
	Pm_Terminate();

	IupClose();

	free(outs);
	return 0;
}

