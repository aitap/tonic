#include "tonic.h"

tone_struct keys[] = {
	{
		.major_name = "C-dur",
		.minor_name = "a-moll",
		.major_tonic = 60,
		.minor_tonic = 57,
	},
	{
		.major_name = "F-dur",
		.minor_name = "d-moll",
		.major_tonic = 65,
		.minor_tonic = 62,
	},
	{
		.major_name = "Bes-dur",
		.minor_name = "g-moll",
		.major_tonic = 70,
		.minor_tonic = 67,
	},
	{
		.major_name = "Es-dur",
		.minor_name = "c-moll",
		.major_tonic = 63,
		.minor_tonic = 60,
	},
	{
		.major_name = "As-dur",
		.minor_name = "f-moll",
		.major_tonic = 68,
		.minor_tonic = 65,
	},
	{
		.major_name = "Des-dur",
		.minor_name = "bes-moll",
		.major_tonic = 61,
		.minor_tonic = 58,
	},
	{
		.major_name = "Ges-dur",
		.minor_name = "es-moll",
		.major_tonic = 67,
		.minor_tonic = 64,
	},
	{
		.major_name = "G-dur",
		.minor_name = "e-moll",
		.major_tonic = 67,
		.minor_tonic = 64,
	},
	{
		.major_name = "D-dur",
		.minor_name = "b-moll",
		.major_tonic = 62,
		.minor_tonic = 59,
	},
	{
		.major_name = "A-dur",
		.minor_name = "fis-moll",
		.major_tonic = 69,
		.minor_tonic = 66,
	},
	{
		.major_name = "E-dur",
		.minor_name = "cis-moll",
		.major_tonic = 64,
		.minor_tonic = 61,
	},
	{
		.major_name = "B-dur",
		.minor_name = "gis-moll",
		.major_tonic = 71,
		.minor_tonic = 68,
	},
	{
		.major_name = "Fis-dur",
		.minor_name = "dis-moll",
		.major_tonic = 66,
		.minor_tonic = 63,
	}
};

size_t keys_size = sizeof(keys)/sizeof(tone_struct);

char* steps[] = { "I", "II", "III", "IV", "V", "VI", "VII" };
size_t steps_size = sizeof(steps)/sizeof(char*);

int major_semitones[7] = {0, 2, 4, 5, 7, 9, 11};
int minor_semitones[7] = {0, 2, 3, 5, 7, 8, 10};
