#pragma once

#include "stdafx.h"
#include "Guess.h"

struct Word
{
	WCHAR chars[6];
	byte impossible; // 0 means the word may be the answer
	byte unusedFlag1;
	byte unusedFlag2;
	byte unusedFlag3;

	int countOccurrences(char letter);
};

// Global data

extern Word* possibleGuesses;
extern int totalPossibleGuesses;
extern int numPossibleGuesses;
extern Word* possibleAnswers;
extern int totalPossibleAnswers;
extern int numPossibleAnswers;
extern int includeGuesses;

// Function prototypes

bool init_words();
void delete_words();
bool loadWords(char* fileName, Word** arr, int* totalWords);
int countWords(FILE* file);
void fillWords(FILE* file, Word* arr, int totalWords);
bool inList(Guess guess, Word* list, int totalWords);