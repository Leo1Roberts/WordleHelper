#pragma once

#include "stdafx.h"
#include <algorithm>

const int MAX_GUESSES = 6;

enum LetterState
{
	LS_EMPTY,
	LS_UNKNOWN,
	LS_INCORRECT,
	LS_MISPLACED,
	LS_CORRECT,
	LS_INCORRECT_INVALID,
	LS_MISPLACED_INVALID,
	LS_CORRECT_INVALID,
	LS_NUM
};

struct Letter
{
	union
	{
		struct
		{
			WCHAR ch;
			WCHAR zero; // Must be left so ch can be referred to as a null-terminated string
		};

		WCHAR string[2];
	};

	LetterState state;

	void empty();
};

struct Guess
{
	Letter letters[5];

	int countOccurrences(WCHAR letter);
};

// Global data

extern Guess guesses[];
extern int currentLine;
extern int currentLetter;

// Function prototypes

void init_guesses();