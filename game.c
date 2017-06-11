#include "tonic.h"
static const int period = 1250; /* ms */

static int uniform_random(int min, int max) {
	/* according to Numerical Recipes in C, applying % to rand() may damage uniformness
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

static void sound_chord(struct game * game, int note, bool single_note) {
	PmTimestamp now = my_timer(NULL);
	PmEvent chord[6];
	// if checkbox is set, play single notes instead of triads
	int max_note = single_note ? 1 : 3;
	for (int i = 0; i < max_note; i++) {
		int abs_note =
			( /* first, get the absolute tonic number */
				game->minor ?
				keys[game->key].minor_tonic :
				keys[game->key].major_tonic
			)
			/* add the offset of the (0..6)th note of the key */
			/* which could be major/minor */
			+ (game->minor ? minor_semitones : major_semitones)[(note + i*2)%7]
			+ 12 * ( (note+i*2)/7 ); /* finally, add an octave to notes which are above 7th note */
		chord[i].timestamp = now;
		chord[i].message = Pm_Message(0x90, abs_note, 100);
		chord[i+max_note].timestamp = now+period;
		chord[i+max_note].message = Pm_Message(0x80, abs_note, 100);
	}
	show_if_pm_error(Pm_Write(game->midi, chord, max_note*2));
}

static bool current_single_note() {
	return (bool)IupGetInt(
		IupGetHandle("single_note_checkbox"),
		"VALUE"
	);
}

static void check_guess(int pressed) {
	struct game * game = (struct game*)IupGetGlobal("struct_game");
	Ihandle *chord_text = IupGetHandle("chord_text");
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
	
	if (guess-1 == game->note)
		IupSetAttribute(chord_text, "FGCOLOR", "#00AA00");
	else 
		IupSetAttribute(chord_text, "FGCOLOR", "#AA0000");

	IupSetAttribute(chord_text, "TITLE", steps[game->note]);
	game->note = uniform_random(0,steps_size-1);
	sound_chord(game, game->note, current_single_note());
}

void change_key(struct game* game) {
	Ihandle *key_text=IupGetHandle("key_text"), *chord_text=IupGetHandle("chord_text");
	game->key = uniform_random(0,keys_size-1);
	game->minor = uniform_random(0,1);
	game->note = 0;
	IupSetAttribute(key_text,
		"TITLE",
		game->minor ?
			keys[game->key].minor_name :
			keys[game->key].major_name
	);
	IupSetAttribute(chord_text,"TITLE",steps[0]);
	IupSetAttribute(chord_text,"FGCOLOR","#000000");
	sound_chord(game, game->note, false);
}

int keypress_callback(Ihandle* dialog, int pressed) {
	struct game * game = (struct game*)IupGetGlobal("struct_game");
	if (iup_isCtrlXkey(pressed)) { check_guess(pressed); }
	else {
		switch (pressed) {
			case K_minus:
				change_key((struct game*)IupGetGlobal("struct_game"));
				break;
			case K_equal:
				sound_chord(game, game->note, current_single_note());
				break;
			case K_t:
				sound_chord(game, 0, false);
				break;
		}
	}
	return IUP_DEFAULT;
}
