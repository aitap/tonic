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
		rand() * (max-min+1.)
		/(RAND_MAX + 1.)
	);
}

void sound_chord(struct game * game, int note, bool single_note) {
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

// note: guess should be int in [1..7]
void check_guess(struct game * game, int guess) {
	Ihandle *chord_text = IupGetHandle("chord_text");
	// clean the buttons
	for (size_t i = 0; i < steps_size; i++) {
		Ihandle * button = IupGetHandle(steps[i]);
		IupSetAttribute(button,"FGCOLOR",IupGetAttribute(button,"DLGFGCOLOR"));
	}

	// show the right answer
	IupSetAttribute(chord_text, "TITLE", steps[game->note]);

	// check and update
	if (guess-1 == game->note) {
		IupSetAttribute(chord_text, "FGCOLOR", "#00AA00");
		// update to a different note
		int old = game->note;
		do
			game->note = uniform_random(0,steps_size-1);
		while (old == game->note);
	} else {
		IupSetAttribute(chord_text, "FGCOLOR", "#AA0000");
		IupSetAttribute(IupGetHandle(steps[guess-1]),"FGCOLOR","#AA0000");
		IupSetAttribute(IupGetHandle(steps[game->note]),"FGCOLOR","#00AA00");
		// don't update the challenge
	}

	// play the next challenge
	sound_chord(game, game->note, game->single);
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
