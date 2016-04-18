#include "tonic.h"
int current_key = 0;
int current_minor = 0;
int current_note = 0;

const int period = 1250; /* ms */

void sound_chord(void) {
	PmTimestamp now = my_timer(NULL);
	PmEvent chord[6];
	for (int i = 0; i < 3; i++) {
		int note = 
			(
				current_minor ?
				keys[current_key].minor_tonic :
				keys[current_key].major_tonic
			) + ((current_note + i*2)%7)[
				current_minor ? minor_semitones : major_semitones
			];
		chord[i].timestamp = now;
		chord[i].message = Pm_Message(0x90, note, 100);
		chord[i+3].timestamp = now+period;
		chord[i+3].message = Pm_Message(0x80, note, 100);
	}
	Pm_Write(midi, chord, 6);
}

void check_guess(int pressed) {
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

	if (!guess) return;
	
	if (guess-1 == current_note)
		IupSetAttribute(chord_text, "FGCOLOR", "#00AA00");
	else 
		IupSetAttribute(chord_text, "FGCOLOR", "#AA0000");

	IupSetAttribute(chord_text, "TITLE", steps[current_note]);
	current_note = rand()%steps_size;
	sound_chord();
}

void change_key(void) {
	current_key = rand()%keys_size;
	current_minor = rand()%2;
	current_note = 0;
	IupSetAttribute(key_text,
		"TITLE",
		current_minor ?
			keys[current_key].minor_name :
			keys[current_key].major_name
	);
	IupSetAttribute(chord_text,"TITLE",steps[0]);
	IupSetAttribute(chord_text,"FGCOLOR","#000000");
	sound_chord();
}

int keypress_callback(Ihandle* dialog, int pressed) {
	if (iup_isCtrlXkey(pressed)) { check_guess(pressed); }
	else {
		switch (pressed) {
			case K_minus:
				change_key();
				break;
			case K_0:
				sound_chord();
				break;
		}
	}
	return IUP_DEFAULT;
}
