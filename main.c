#include "tonic.h"
/* the icon */
extern unsigned char tonic_64x64_rgba[];

static const int32_t latency = 16; /* I need timestamps, and 16ms seems reasonable */

PmError show_if_pm_error(PmError code) {
	static const size_t err_msg_len = 1024;
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

static int set_single_notes(Ihandle* checkbox, int set) {
	struct game * game = (struct game*)IupGetAttribute(checkbox,"struct_game");
	game->single = (bool)IupGetInt(checkbox,"VALUE");
	return IUP_DEFAULT;
}

static int answer_button_callback(Ihandle* button) {
	struct game * game = (struct game*)IupGetAttribute(button,"struct_game");
	int answer = IupGetInt(button,"answer");
	check_guess(game,answer);
	return IUP_DEFAULT;
}

static int again_button_callback(Ihandle* button) {
	struct game * game = (struct game*)IupGetAttribute(button,"struct_game");
	sound_chord(game, game->note, game->single);
	return IUP_DEFAULT;
}

static int tonic_button_callback(Ihandle* button) {
	struct game * game = (struct game*)IupGetAttribute(button,"struct_game");
	sound_chord(game, 0, false);
	return IUP_DEFAULT;
}

static int key_button_callback(Ihandle* button) {
	struct game * game = (struct game*)IupGetAttribute(button,"struct_game");
	change_key(game);
	return IUP_DEFAULT;
}

static int pressed_to_guess(int pressed) {
	int guess = 0;
	switch (pressed) {
		case iup_XkeyCtrl(K_1): guess = 1; break;
		case iup_XkeyCtrl(K_2): guess = 2; break;
		case iup_XkeyCtrl(K_3): guess = 3; break;
		case iup_XkeyCtrl(K_4): guess = 4; break;
		case iup_XkeyCtrl(K_5): guess = 5; break;
		case iup_XkeyCtrl(K_6): guess = 6; break;
		case iup_XkeyCtrl(K_7): guess = 7; break;
	}
	return guess;
}

static int keypress_callback(Ihandle* dialog, int pressed) {
	struct game * game = (struct game*)IupGetAttribute(dialog,"struct_game");
	if (iup_isCtrlXkey(pressed)) {
		int guess = pressed_to_guess(pressed);
		if (guess) check_guess(game,guess);
	} else {
		switch (pressed) {
			case K_minus:
				change_key(game);
				break;
			case K_equal:
				sound_chord(game, game->note, game->single);
				break;
			case K_t:
				sound_chord(game, 0, false);
				break;
		}
	}
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

	struct game game = {
		.midi=NULL, .outs=NULL, // to be filled later
		.key=0, .note=0, .minor=false, // safe defaults
		.single=true // this *is* default
	};

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
			size_t length = snprintf(NULL, 0, "%d", (int)j)+1;
			char* att_name = malloc(length);
			assert(att_name); // failure to allocate 2..3 bytes should be fatal (does anyone have 1000 MIDI outputs?)
			snprintf(att_name, length, "%d", (int)j+1);
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

	Ihandle * answer_row = IupHbox(NULL);
	for (size_t i = 0; i < steps_size; i++) {
		Ihandle * button = IupSetAtt(
			steps[i],IupButton(steps[i],NULL), // we'll use the names to color the buttons
			"EXPAND","HORIZONTAL","ALIGNMENT","ACENTER:ACENTER",
			NULL
		);
		IupSetInt(button,"answer",(int)i+1);
		IupSetCallback(button,"ACTION",answer_button_callback);
		IupAppend(answer_row, button);
	}

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
						IupSetCallbacks(
							IupSetAttributes(IupToggle("&Single notes",NULL),"VALUE=ON"), // reflect the default in struct game
							"ACTION",(Icallback)set_single_notes,
							NULL
						),
						IupHorizExpand(IupAlignCenter(key_text)), IupHorizExpand(IupAlignCenter(chord_text)),
						answer_row,
						IupHbox(
							IupSetCallbacks(IupHorizExpand(IupButton("&Again",NULL)),"ACTION",again_button_callback,NULL),
							IupSetCallbacks(IupHorizExpand(IupButton("&Tonic",NULL)),"ACTION",tonic_button_callback,NULL),
							IupSetCallbacks(IupHorizExpand(IupButton("&Change key",NULL)),"ACTION",key_button_callback,NULL),
							NULL
						),
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

