#pragma once

#include "stdafx.h"
#include "Guess.h"
#include "Button.h"

// Global data

extern HBRUSH br_BACKGROUND;
extern HBRUSH br_UNKNOWN_OUTLINE;
extern HBRUSH br_INCORRECT;
extern HBRUSH br_MISPLACED;
extern HBRUSH br_CORRECT;
extern HBRUSH br_INVALID;
extern HBRUSH br_BAD_SUGGESTION;
extern int suggestionHeight;
extern int suggestionWidth;
extern int suggestionGapY;
extern int suggestionGapX;
extern int suggestionSpaceHeight;
extern int suggestionTopOffset;
extern int suggestionTotalHeight;
extern int scrollbarBackingWidth;

// Function prototypes

void init_graphics();
void delete_graphics();
void paint(HWND hWnd);
void createGuessGrid(HDC hdc, RECT space);
void createSuggestionList(HDC hdc, RECT space);