#include "stdafx.h"
#include "Button.h"
#include "Guess.h"
#include "Graphics.h"
#include "Control.h"

Button allButtons[MAX_BUTTONS];
int nextButton;
Button* mouseOverButton = NULL;
int mouseY;
int intialSuggestionTopOffset;
bool dragging = false;

void addButton(const RECT* rect, ButtonType type, const WCHAR* str, int data1, int data2, bool mouseOver)
{
	if (nextButton == MAX_BUTTONS) return;

	Button* fill = allButtons + nextButton;

	fill->rect = *rect;
	fill->type = type;
	fill->str = str;
	fill->data1 = data1;
	fill->data2 = data2;
	fill->mouseOver = mouseOver;

	nextButton++;
}

void drawButtons(HDC hdc, ButtonType type)
{
	// Rectangles

	for (int i = 0; i < nextButton; i++)
	{
		Button* bt = allButtons + i;
		if (bt->type != type) continue;

		switch (type)
		{
		case BT_LETTER:
		{
			HBRUSH brush = br_INVALID;
			HBRUSH innerBrush = br_INVALID;

			LetterState state = guesses[bt->data1].letters[bt->data2].state;

			bool border = true;

			switch (state)
			{
			case LS_EMPTY:
				brush = br_INCORRECT;
				innerBrush = br_BACKGROUND;
				break;
			case LS_UNKNOWN:
				brush = br_UNKNOWN_OUTLINE;
				innerBrush = br_BACKGROUND;
				break;
			case LS_INCORRECT:
				brush = br_INCORRECT;
				border = false;
				break;
			case LS_MISPLACED:
				brush = br_MISPLACED;
				border = false;
				break;
			case LS_CORRECT:
				brush = br_CORRECT;
				border = false;
				break;
			case LS_INCORRECT_INVALID:
				brush = br_INVALID;
				innerBrush = br_INCORRECT;
				break;
			case LS_MISPLACED_INVALID:
				brush = br_INVALID;
				innerBrush = br_MISPLACED;
				break;
			case LS_CORRECT_INVALID:
				brush = br_INVALID;
				innerBrush = br_CORRECT;
				break;
			}

			FillRect(hdc, &bt->rect, brush);

			if (border)
			{
				RECT rect;
				int border = round((float)(bt->rect.bottom - bt->rect.top) / 31.0f);
				SetRect(&rect, bt->rect.left + border, bt->rect.top + border, bt->rect.right - border, bt->rect.bottom - border);
				FillRect(hdc, &rect, innerBrush);
			}

			break;
		}
		case BT_CLEAR_LINE:
			if (bt->data2 == 1)
			{
				bt->str = L"\u23CE";
				if (bt->mouseOver && !dragging) FillRect(hdc, &bt->rect, br_CORRECT);
				else FillRect(hdc, &bt->rect, br_INCORRECT);
			}
			else if (bt->data2 == 2)
			{
				bt->str = L"×";
				if (bt->mouseOver && !dragging) FillRect(hdc, &bt->rect, br_INVALID);
				else FillRect(hdc, &bt->rect, br_INCORRECT);
			}
			break;
		case BT_SUGGESTION:
			if (bt->mouseOver && !dragging) FillRect(hdc, &bt->rect, br_UNKNOWN_OUTLINE);
			else if (bt->data1) FillRect(hdc, &bt->rect, br_BAD_SUGGESTION);
			else FillRect(hdc, &bt->rect, br_INCORRECT);
			break;
		case BT_SCROLLBAR:
			if (bt->mouseOver || dragging) FillRect(hdc, &bt->rect, br_UNKNOWN_OUTLINE);
			else FillRect(hdc, &bt->rect, br_INCORRECT);
			break;
		}
	}

	// Letters

	SetBkMode(hdc, TRANSPARENT);
	HFONT hFontOriginal, letterFont, clearLineFont, suggestionFont;

	int letterFontSize = 0;
	int clearLineFontSize = 0;
	int suggestionFontSize = 0;
	for (int i = 0; i < nextButton; i++)
	{
		Button* bt = allButtons + i;
		if (bt->type != type) continue;

		switch (type)
		{
		case BT_LETTER:
			letterFontSize = (float)(bt->rect.bottom - bt->rect.top) * 0.7f;
			break;
		case BT_CLEAR_LINE:
			clearLineFontSize = bt->rect.bottom - bt->rect.top;
			break;
		case BT_SUGGESTION:
			suggestionFontSize = (bt->rect.bottom - bt->rect.top) * 0.8f;
			break;
		}
	}

	letterFont = CreateFont(letterFontSize, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Franklin Gothic Demi"));
	clearLineFont = CreateFont(clearLineFontSize, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Franklin Gothic Demi"));
	suggestionFont = CreateFont(suggestionFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Consolas"));

	for (int i = 0; i < nextButton; i++)
	{
		Button* bt = allButtons + i;
		if (bt->type != type) continue;

		switch (type)
		{
		case BT_LETTER:
			hFontOriginal = (HFONT)SelectObject(hdc, letterFont);
			SetTextColor(hdc, RGB(0xF8, 0xF8, 0xF8));
			DrawText(hdc, bt->str, -1, &bt->rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			SelectObject(hdc, hFontOriginal);
			break;
		case BT_CLEAR_LINE:
			if (bt->data2 != 0)
			{
				hFontOriginal = (HFONT)SelectObject(hdc, clearLineFont);
				if (bt->mouseOver) SetTextColor(hdc, RGB(0xF8, 0xF8, 0xF8));
				else SetTextColor(hdc, RGB(0xC0, 0xC0, 0xC0));
				RECT crossRect = bt->rect;
				crossRect.bottom -= (crossRect.bottom - crossRect.top) * 0.08;
				DrawText(hdc, bt->str, -1, &bt->rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				SelectObject(hdc, hFontOriginal);
			}
			break;
		case BT_SUGGESTION:
			hFontOriginal = (HFONT)SelectObject(hdc, suggestionFont);
			SetTextColor(hdc, RGB(0xF8, 0xF8, 0xF8));
			DrawText(hdc, bt->str, -1, &bt->rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			SelectObject(hdc, hFontOriginal);
			break;
		}
	}

	DeleteObject(letterFont);
	DeleteObject(clearLineFont);
	DeleteObject(suggestionFont);
}

bool buttonMovedOver(int x, int y)
{
	Button* bt = NULL;

	for (int i = 0; i < nextButton; i++)
	{
		Button* check = allButtons + i;
		if (x >= check->rect.left && x < check->rect.right && y >= check->rect.top && y < check->rect.bottom) bt = check;
	}

	if (bt == NULL)
	{
		if (mouseOverButton == NULL) return false;

		mouseOverButton->mouseOver = false;
		mouseOverButton = NULL;
		return true;
	}

	if (bt == mouseOverButton) return false;

	if (mouseOverButton != NULL) mouseOverButton->mouseOver = false;

	mouseOverButton = bt;

	switch (bt->type)
	{
	case BT_CLEAR_LINE:
		if (bt->data2 != 0) bt->mouseOver = true;
		break;
	case BT_SUGGESTION:
		bt->mouseOver = true;
		break;
	case BT_SCROLLBAR:
		bt->mouseOver = true;
		break;
	}

	return true;
}

bool buttonClicked(int x, int y, bool leftButton)
{
	Button* bt = NULL;

	for (int i = 0; i < nextButton; i++)
	{
		Button* check = allButtons + i;
		if (x >= check->rect.left && x < check->rect.right && y >= check->rect.top && y < check->rect.bottom) bt = check;
	}

	if (bt == NULL) return false;

	switch (bt->type)
	{
	case BT_LETTER:
	{
		if (bt->data1 != currentLine - 1) return false;

		LetterState* state = &guesses[bt->data1].letters[bt->data2].state;
		switch (*state)
		{
		case LS_INCORRECT:
			*state = leftButton ? LS_MISPLACED : LS_CORRECT;
			break;
		case LS_MISPLACED:
			*state = leftButton ? LS_CORRECT : LS_INCORRECT;
			break;
		case LS_CORRECT:
			*state = leftButton ? LS_INCORRECT : LS_MISPLACED;
			break;
		case LS_INCORRECT_INVALID:
			*state = leftButton ? LS_MISPLACED : LS_CORRECT;
			break;
		case LS_MISPLACED_INVALID:
			*state = leftButton ? LS_CORRECT : LS_INCORRECT;
			break;
		case LS_CORRECT_INVALID:
			*state = leftButton ? LS_INCORRECT : LS_MISPLACED;
			break;
		}

		checkColours();
		updatePossibleAnswers();
		break;
	}
	case BT_CLEAR_LINE:
		if (leftButton)
		{
			if (bt->data2 == 1)
			{
				charTyped('\r');
			}
			else if (bt->data2 == 2)
			{
				for (int line = bt->data1; line <= currentLine; line++)
				{
					for (int let = 0; let < 5; let++)
					{
						guesses[line].letters[let].empty();
					}
				}

				bt->data2 = currentLetter = 0;
				currentLine = bt->data1;

				updatePossibleAnswers();
			}
		}
		break;
	case BT_SUGGESTION:
		if (leftButton)
		{
			for (int let = 0; let < 5; let++)
			{
				charTyped('\b');
			}
			for (int let = 0; let < 5; let++)
			{
				charTyped(*(bt->str + let));
			}
			charTyped('\r');
		}
		break;
	case BT_SCROLLBAR:
		if (leftButton)
		{
			dragging = true;
			mouseY = y;
			intialSuggestionTopOffset = suggestionTopOffset;
		}
		break;
	}

	return true;
}

bool scrollbarDragging(int y)
{
	if (!dragging) return false;

	suggestionTopOffset = intialSuggestionTopOffset + round((float)(mouseY - y) * (float)suggestionTotalHeight / (float)suggestionSpaceHeight);
	return true;
}

bool scrollbarDraggingEnd()
{
	if (!dragging) return false;

	dragging = false;
	return true;
}