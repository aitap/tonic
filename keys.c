#include "tonic.h"

tone_struct keys[] = {
	{
		.major_name = "C-dur",
		.minor_name = "a-moll",
		.major_tonic = 60,
		.minor_tonic = 57,
		.signature = {0, 0, 0, 0, 0, 0, 0}
	}
};

size_t keys_size = sizeof(keys)/sizeof(tone_struct);

char* steps[] = { "I", "II", "III", "IV", "V", "VI", "VII" };
size_t steps_size = sizeof(steps)/sizeof(char*);

int major_semitones[7] = {0, 2, 4, 5, 7, 9, 11};
int minor_semitones[7] = {0, 2, 3, 5, 7, 8, 10};
