#include "tonic.h"
int current_key = 0;
int current_note = 0;

void check_guess(int pressed) {
	if (!current_note) return;

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

/*	if (guess != current_note) {
		IupSetAttribute*/
}

int keypress_callback(Ihandle* dialog, int pressed) {
	if (iup_isCtrlXkey(pressed)) { check_guess(pressed); }
	else {
		switch (pressed) {
			case K_CR:
//				change_key();
				break;
			case K_SP:
//				sound_chord();
				break;
		}
	}
	return IUP_DEFAULT;
}
