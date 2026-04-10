#include "stdafx.h"
#include "Guess.h"

Guess guesses[MAX_GUESSES];
int currentLine;
int currentLetter;

void init_guesses()
{
	for (int line = 0; line < MAX_GUESSES; line++) for (int letter = 0; letter < 5; letter++) guesses[line].letters[letter].empty();
}

void Letter::empty()
{
	ch = 0;
	state = LS_EMPTY;
}

int Guess::countOccurrences(WCHAR letter)
{
	int count = 0;

	for (int i = 0; i < 5; i++) if (letters[i].ch == letter) count++;

	return count;
}