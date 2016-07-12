#include "tonic.h"
int current_key = 0;
bool current_minor = false;
int current_note = 0;

const int period = 1250; /* ms */

int uniform_random(int min, int max) {
	/* according to Numerical Recipes in C, applying % to rand() damages uniformness
	 * rand/RAND_MAX => [0,1], with 1.0 being very rare
	 * rand/(RAND_MAX+1) => [0,1)
	 * (max-min+1)*rand/(RAND_MAX+1) => [0,max-min+1), as uniform as rand itself
	 * (int) transforms that into uniformly distributed [0,max-min]
	 * + min => [min,max]
	 */
	return min + (int)(
		rand() * (double)(max-min+1)
		/(RAND_MAX + 1.0)
	);
}

void sound_chord(int key, int note, bool minor, bool single_note) {
	PmTimestamp now = my_timer(NULL);
	PmEvent chord[6];
	// if checkbox is set, play single notes instead of triads
	int max_note = single_note ? 1 : 3;
	for (int i = 0; i < max_note; i++) {
		int abs_note =
			( /* first, get the absolute tonic number */
				minor ?
				keys[key].minor_tonic :
				keys[key].major_tonic
			) + ((note + i*2)%7)[ /* add the offset of the (0..6)th note of the key */
				minor ? minor_semitones : major_semitones /* which could be major/minor */
			] + 12 * ( (note+i*2)/7 ); /* finally, add an octave to notes which are above 7th note */
		chord[i].timestamp = now;
		chord[i].message = Pm_Message(0x90, abs_note, 100);
		chord[i+max_note].timestamp = now+period;
		chord[i+max_note].message = Pm_Message(0x80, abs_note, 100);
	}
	show_if_pm_error(Pm_Write(midi, chord, max_note*2));
}

bool current_single_note() {
	return (bool)IupGetInt(single_note_checkbox,"VALUE");
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
	current_note = uniform_random(0,steps_size-1);
	sound_chord(current_key, current_note, current_minor, current_single_note());
}

void change_key(void) {
	current_key = uniform_random(0,keys_size-1);
	current_minor = uniform_random(0,1);
	current_note = 0;
	IupSetAttribute(key_text,
		"TITLE",
		current_minor ?
			keys[current_key].minor_name :
			keys[current_key].major_name
	);
	IupSetAttribute(chord_text,"TITLE",steps[0]);
	IupSetAttribute(chord_text,"FGCOLOR","#000000");
	sound_chord(current_key, current_note, current_minor, false);
}

int keypress_callback(Ihandle* dialog, int pressed) {
	if (iup_isCtrlXkey(pressed)) { check_guess(pressed); }
	else {
		switch (pressed) {
			case K_minus:
				change_key();
				break;
			case K_equal:
				sound_chord(current_key, current_note, current_minor, current_single_note());
				break;
			case K_t:
				sound_chord(current_key, 0, current_minor, 0);
				break;
		}
	}
	return IUP_DEFAULT;
}
