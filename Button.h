#pragma once

#include "stdafx.h"

const int MAX_BUTTONS = 15000;

enum ButtonType
{
	BT_LETTER,
	BT_CLEAR_LINE,
	BT_SUGGESTION,
	BT_SCROLLBAR,
	BT_NUM
};

struct Button
{
	RECT rect;
	ButtonType type;
	const WCHAR* str;
	int data1, data2;
	bool mouseOver;
};

// Global data

extern Button allButtons[];
extern int nextButton;
extern Button* mouseOverButton;
extern int mouseY;
extern int intialSuggestionTopOffset;

// Function prototypes

void addButton(const RECT* rect, ButtonType type, const WCHAR* str, int data1, int data2, bool mouseOver);
void drawButtons(HDC hdc, ButtonType type);
bool buttonMovedOver(int x, int y);
bool buttonClicked(int x, int y, bool leftButton);
bool scrollbarDragging(int y);
bool scrollbarDraggingEnd();