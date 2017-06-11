#include "tonic.h"
/* the icon */
extern unsigned char tonic_64x64_rgba[];

static const int32_t latency = 16; /* I need timestamps, and 16ms seems reasonable */
static const size_t err_msg_len = 1024;

PmError show_if_pm_error(PmError code) {
	char os_error[err_msg_len];
	if (code == pmNoError) return code;
	if (code == pmHostError)
		Pm_GetHostErrorText(os_error, err_msg_len);
	IupMessage("PortMidi error", code == pmHostError ? os_error : Pm_GetErrorText(code));
	return code;
}

PmTimestamp my_timer (void* unused) {
	(void)unused; /* Pt_Time doesn't need any arguments, but PmTimeProc does */
	return (PtTimestamp)Pt_Time();
}

static int open_audio_callback(Ihandle* button) {
	struct game * game = (struct game*)IupGetAttribute(button,"struct_game");

	if (
		game->midi &&
		(show_if_pm_error(Pm_Close(game->midi)) != pmNoError)
	) return IUP_DEFAULT;

	game->midi=NULL;

	size_t out_number = (size_t)IupGetInt((Ihandle*)IupGetAttribute(button,"device_list"), "VALUE");
	if (!out_number) return IUP_DEFAULT; // can be zero if there is no selected item, otherwise belongs to 1..N
	
	show_if_pm_error(Pm_OpenOutput(
		&game->midi /* struct to work with */,
		game->outs[out_number-1] /* number of MIDI output */,
		NULL /* optional driver-specific wtf */,
		0 /* apparently I don't need to buffer */,
		(PmTimeProcPtr)my_timer /* adapt porttime to portmidi's function pointer requirements */,
		NULL /* no arguments to timing function, either */,
		latency
	));

	/* PortMidi seems to return error codes if given a NULL, so no abort() here */
	return IUP_DEFAULT;
}

static int set_program_callback(Ihandle* button) {
	show_if_pm_error(
		Pm_WriteShort(
			((struct game*)IupGetAttribute(button,"struct_game"))->midi,
			0,
			Pm_Message(
				0xC0,
				IupGetInt((Ihandle*)IupGetAttribute(button,"program_number"),"VALUE"),
				0
			)
		)
	);
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
	/* note: the former was a PtError, not a PmError */
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
	/* at this point we should have at least one output device */

	struct game game = {};

	Ihandle * device_list = IupList(NULL); /* ask the user which output device they want */

	{
		size_t num_outs = 0;
		size_t num_devices = (size_t)Pm_CountDevices(); // docs state that ids range from 0 to Pm_CountDevices()-1
		game.outs = calloc(num_devices, sizeof(PmDeviceID)); // outs <= all devices, so some of these will be unused
		assert(game.outs);
	
		for (PmDeviceID i = 0; i < num_devices; i++) {
			if (Pm_GetDeviceInfo(i)->output) {
				game.outs[num_outs] = i;
				num_outs++;
			}
		}
		/* at this point valid device IDs are in range 0..num_outs-1 */

		for (size_t j = 0; j < num_outs; j++) {
			size_t length = snprintf(NULL, 0, "%d", j)+1;
			char* att_name = malloc(length);
			assert(att_name); // failure to allocate 2..3 bytes should be fatal (does anyone have 1000 MIDI outputs?)
			snprintf(att_name, length, "%d", j+1);
			IupSetAttribute(device_list, att_name, Pm_GetDeviceInfo(game.outs[j])->name); // return values of GetDeviceInfo persist until Pm_Terminate()
			free(att_name);
			if (out == game.outs[j]) IupSetInt(device_list, "VALUE", j+1); // set the dropdown to currently selected output
		}
	}
	IupSetAttribute(device_list, "DROPDOWN", "YES");
	Ihandle * open_audio_button = IupSetCallbacks(
		IupSetAtt(
			NULL,IupButton("Open",NULL),
			"device_list",device_list,
			NULL
		),
		"ACTION",(Icallback)open_audio_callback,
		NULL
	);

	Ihandle * program_number = IupSetAttributes(IupText(NULL),"VALUE=0,SPIN=YES,SPINMIN=0,SPINMAX=255,SPININC=1");
	IupSetAttribute(program_number, "MASK", IUP_MASK_INT);
	Ihandle * program_number_button = IupSetCallbacks(
		IupSetAtt(
			NULL,IupButton("Set program",NULL),
			"program_number",program_number,
			NULL
		),
		"ACTION",(Icallback)set_program_callback,
		NULL
	);

	Ihandle * key_text = IupLabel("___-____");
	IupSetHandle("key_text",key_text);
	IupSetInt(key_text,"FONTSIZE",IupGetInt(key_text,"FONTSIZE")*2);
	Ihandle * chord_text = IupLabel("__");
	IupSetHandle("chord_text",chord_text);
	IupSetInt(chord_text,"FONTSIZE",IupGetInt(chord_text,"FONTSIZE")*4);

#define IupHorizExpand(x) IupSetAttributes((x),"EXPAND=HORIZONTAL")
#define IupAlignCenter(x) IupSetAttributes((x),"ALIGNMENT=ACENTER:ACENTER")
	IupShow(
		IupSetAtt(
			NULL,
			IupSetCallbacks(
				IupDialog(
					IupVbox(
						IupHbox(IupHorizExpand(device_list),open_audio_button,NULL),
						IupHbox(
							IupHorizExpand(program_number),
							program_number_button,
						NULL),
						/* this should have been included in struct game, but storing state in the Ihandle yields more compact code - maybe refactor later */
						IupSetAtt("single_note_checkbox",IupToggle("&Single notes",NULL),"VALUE","ON",NULL),
						IupHorizExpand(IupAlignCenter(key_text)), IupHorizExpand(IupAlignCenter(chord_text)),
						IupHorizExpand(IupAlignCenter(IupLabel("guess: ctrl+1..7\nplay chord again: =\nplay tonic again: t\nnew key: -"))),
						NULL
					)
				),
				"K_ANY",(Icallback)keypress_callback,
				NULL
			),
			"TITLE","Tonic","RESIZE","NO",
			"ICON",IupImageRGBA(64,64,tonic_64x64_rgba),
			"struct_game",(void*)&game,
			NULL
		)
	);

	// everything is set up => try to open out device
	open_audio_callback(open_audio_button);
	// set the key, too
	srand((unsigned int)time(NULL));
	change_key(&game);

	IupMainLoop();

	Pm_Close(game.midi);

	// IUP has stored some pointers owned by PortMidi => shut down the former first
	IupClose();
	Pm_Terminate();
	free(game.outs);
	return 0;
}

