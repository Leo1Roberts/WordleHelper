#include "stdafx.h"
#include "Graphics.h"
#include "Words.h"

HBRUSH br_BACKGROUND;
HBRUSH br_UNKNOWN_OUTLINE;
HBRUSH br_INCORRECT;
HBRUSH br_MISPLACED;
HBRUSH br_CORRECT;
HBRUSH br_INVALID;
HBRUSH br_BAD_SUGGESTION;
HBRUSH br_BLACK;
HBRUSH br_BACKGROUND_DARK;
int suggestionHeight;
int suggestionWidth;
int suggestionGapY;
int suggestionGapX;
int suggestionSpaceHeight;
int suggestionTopOffset;
int suggestionTotalHeight;
int scrollbarBackingWidth;

void init_graphics()
{
	br_BACKGROUND = CreateSolidBrush(RGB(0x12, 0x12, 0x13));
	br_UNKNOWN_OUTLINE = CreateSolidBrush(RGB(0x56, 0x57, 0x58));
	br_INCORRECT = CreateSolidBrush(RGB(0x3A, 0x3A, 0x3C));
	br_MISPLACED = CreateSolidBrush(RGB(0xB5, 0x9F, 0x3B));
	br_CORRECT = CreateSolidBrush(RGB(0x53, 0x8D, 0x4E));
	br_INVALID = CreateSolidBrush(RGB(0xc0, 0x00, 0x00));
	br_BAD_SUGGESTION = CreateSolidBrush(RGB(0x25, 0x25, 0x26));
	br_BLACK = CreateSolidBrush(RGB(0x00, 0x00, 0x00));
	br_BACKGROUND_DARK = CreateSolidBrush(RGB(0x09, 0x09, 0x09));
	suggestionHeight = GetSystemMetrics(SM_CYSCREEN) / 40;
	suggestionWidth = suggestionHeight * 2.3f;
	suggestionGapY = suggestionHeight / 5;
	suggestionGapX = max(1, min(suggestionGapY - 1, suggestionGapY * 0.8f));
	scrollbarBackingWidth = GetSystemMetrics(SM_CYSCREEN) / 50;
}

void delete_graphics()
{
	DeleteObject(br_BACKGROUND);
	DeleteObject(br_UNKNOWN_OUTLINE);
	DeleteObject(br_INCORRECT);
	DeleteObject(br_MISPLACED);
	DeleteObject(br_CORRECT);
}

void paint(HWND hWnd)
{
	// Setup

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	HDC hMemDC = CreateCompatibleDC(hdc);

	int width = ps.rcPaint.right - ps.rcPaint.left;
	int height = ps.rcPaint.bottom - ps.rcPaint.top;
	POINT oldOrigin;

	HBITMAP hBmp = CreateCompatibleBitmap(hdc, width, height);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hBmp);

	SetViewportOrgEx(hMemDC, 0, 0, &oldOrigin);

	// Painting

	nextButton = 0;

	RECT rect;
	GetClientRect(hWnd, &rect);
	int rectWidth = rect.right - rect.left;
	int rectHeight = rect.bottom - rect.top;

	RECT guessGridBacking;
	RECT guessGridSpace;
	RECT suggestionBacking;
	RECT suggestionSpace;
	RECT scrollbarBacking;
	RECT scrollbarSpace;
	int divide;

	if (rectWidth >= rectHeight) // Window is square or wider
	{
		if (rectWidth < rectHeight + suggestionWidth + 2 * suggestionGapX + scrollbarBackingWidth) divide = rect.right - suggestionWidth - 2 * suggestionGapX - scrollbarBackingWidth;
		else divide = rect.left + rectHeight;

		SetRect(&guessGridBacking, rect.left, rect.top, divide, rect.bottom);
		SetRect(&guessGridSpace, guessGridBacking.left + rectHeight / 20, guessGridBacking.top + rectHeight / 20, guessGridBacking.right - rectHeight / 20, guessGridBacking.bottom - rectHeight / 20);
		SetRect(&suggestionBacking, divide, rect.top, rect.right - scrollbarBackingWidth, rect.bottom);
		SetRect(&suggestionSpace, suggestionBacking.left + suggestionGapX, suggestionBacking.top + suggestionGapY, suggestionBacking.right - suggestionGapX, suggestionBacking.bottom - suggestionGapY);
		suggestionSpaceHeight = suggestionSpace.bottom - suggestionSpace.top;
		SetRect(&scrollbarBacking, rect.right - scrollbarBackingWidth, rect.top, rect.right, rect.bottom);
		SetRect(&scrollbarSpace, scrollbarBacking.left + suggestionGapX, scrollbarBacking.top + suggestionGapY, scrollbarBacking.right - suggestionGapX, scrollbarBacking.bottom - suggestionGapY);
	}
	else
	{
		if (rectHeight < rectWidth + suggestionHeight + 2 * suggestionGapY) divide = rect.bottom - suggestionHeight - 2 * suggestionGapY;
		else divide = rect.top + rectWidth;

		SetRect(&guessGridBacking, rect.left, rect.top, rect.right, divide);
		SetRect(&guessGridSpace, guessGridBacking.left + rectWidth / 20, guessGridBacking.top + rectWidth / 20, guessGridBacking.right - rectWidth / 20, guessGridBacking.bottom - rectWidth / 20);
		SetRect(&suggestionBacking, rect.left, divide, rect.right - scrollbarBackingWidth, rect.bottom);
		SetRect(&suggestionSpace, suggestionBacking.left + suggestionGapX, suggestionBacking.top + suggestionGapY, suggestionBacking.right - suggestionGapX, suggestionBacking.bottom - suggestionGapY);
		suggestionSpaceHeight = suggestionSpace.bottom - suggestionSpace.top;
		SetRect(&scrollbarBacking, rect.right - scrollbarBackingWidth, divide, rect.right, rect.bottom);
		SetRect(&scrollbarSpace, scrollbarBacking.left + suggestionGapX, scrollbarBacking.top + suggestionGapY, scrollbarBacking.right - suggestionGapX, scrollbarBacking.bottom - suggestionGapY);
	}

	createGuessGrid(hMemDC, guessGridSpace);
	createSuggestionList(hMemDC, suggestionSpace);
	FillRect(hMemDC, &suggestionBacking, br_BLACK);
	FillRect(hMemDC, &scrollbarBacking, br_BACKGROUND_DARK);
	drawButtons(hMemDC, BT_SUGGESTION);
	drawButtons(hMemDC, BT_SCROLLBAR);
	FillRect(hMemDC, &guessGridBacking, br_BACKGROUND);
	drawButtons(hMemDC, BT_LETTER);
	drawButtons(hMemDC, BT_CLEAR_LINE);
	//drawAllButtons(hMemDC);

	// Restoration

	SetViewportOrgEx(hMemDC, oldOrigin.x, oldOrigin.y, NULL);

	BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);

	//sprintf_s(msg, 1024, "ps.rcPaint l %d / r %d / t %d / b %d\n", ps.rcPaint.left, ps.rcPaint.right, ps.rcPaint.top, ps.rcPaint.bottom);
	//OutputDebugStringA(msg);

	SelectObject(hMemDC, hOldBmp);
	DeleteObject(hBmp);
	DeleteDC(hMemDC);

	EndPaint(hWnd, &ps);
}

void createGuessGrid(HDC hdc, RECT space)
{
	RECT rect;

	int limitingDim = min(space.right - space.left, space.bottom - space.top);

	int gap = (int)((float)(limitingDim) / 82.0f);
	int size = (int)((float)(limitingDim - gap * 5) / 6.0f);

	for (int line = 0; line < MAX_GUESSES; line++)
	{
		for (int letter = 0; letter < 6; letter++)
		{
			int x = space.left + (space.right - space.left - (gap * 5 + size * 6)) / 2 + (size + gap) * letter;
			int y = space.top + (space.bottom - space.top - (gap * 5 + size * 6)) / 2 + (size + gap) * line;

			if (letter == 5)
			{
				SetRect(&rect, x + size / 4, y + size / 4, x + size - size / 4, y + size - size / 4);
				int data2 = 0;
				if (line < currentLine) data2 = 2;
				else if (line == currentLine && currentLetter == 5) data2 = 1;
				addButton(&rect, BT_CLEAR_LINE, L"×", line, data2, allButtons[nextButton].mouseOver);
			}
			else
			{
				SetRect(&rect, x, y, x + size, y + size);
				addButton(&rect, BT_LETTER, guesses[line].letters[letter].string, line, letter, allButtons[nextButton].mouseOver);
			}
		}
	}
}

void createSuggestionList(HDC hdc, RECT space)
{
	RECT rect;

	int spaceWidth = space.right - space.left;
	int spaceHeight = suggestionSpaceHeight;

	int suggestionCount = 0;
	int x, y;
	int columns = max(1, (spaceWidth + suggestionGapX) / (suggestionWidth + suggestionGapX));

	suggestionTotalHeight = max(1, ceil((float)(numPossibleAnswers + numPossibleGuesses * includeGuesses) / (float)columns) * (suggestionHeight + suggestionGapY) - suggestionGapY);
	suggestionTopOffset = min(0, max(suggestionTopOffset, spaceHeight - suggestionTotalHeight));

	int scrollbarSpaceHeight = space.bottom - space.top;
	int scrollbarHeight = max(1, min(scrollbarSpaceHeight, (float)scrollbarSpaceHeight * (float)suggestionSpaceHeight / (float)suggestionTotalHeight));
	int scrollbarTop = space.top - (float)suggestionTopOffset / (float)suggestionTotalHeight * (float)scrollbarSpaceHeight;
	SetRect(&rect, space.right + 2 * suggestionGapX, scrollbarTop, space.right + scrollbarBackingWidth, scrollbarTop + scrollbarHeight);
	addButton(&rect, BT_SCROLLBAR, L"", 0, 0, allButtons[nextButton].mouseOver);

	for (int i = 0; i < totalPossibleAnswers; i++)
	{
		if (!possibleAnswers[i].impossible)
		{
			x = space.left + (spaceWidth + suggestionGapX - columns * (suggestionWidth + suggestionGapX)) / 2 + (suggestionCount % columns) * (suggestionWidth + suggestionGapX);
			y = space.top + suggestionTopOffset + (suggestionCount / columns) * (suggestionHeight + suggestionGapY);

			if (y + suggestionHeight > space.top && y < space.bottom)
			{
				SetRect(&rect, x, y, x + suggestionWidth, y + suggestionHeight);
				addButton(&rect, BT_SUGGESTION, possibleAnswers[i].chars, 0, 0, allButtons[nextButton].mouseOver);
			}

			suggestionCount++;
		}
	}

	if (includeGuesses)
	{
		for (int i = 0; i < totalPossibleGuesses; i++)
		{
			if (!possibleGuesses[i].impossible)
			{
				x = space.left + (spaceWidth + suggestionGapX - columns * (suggestionWidth + suggestionGapX)) / 2 + (suggestionCount % columns) * (suggestionWidth + suggestionGapX);
				y = space.top + suggestionTopOffset + (suggestionCount / columns) * (suggestionHeight + suggestionGapY);

				if (y + suggestionHeight > space.top && y < space.bottom)
				{
					SetRect(&rect, x, y, x + suggestionWidth, y + suggestionHeight);
					addButton(&rect, BT_SUGGESTION, possibleGuesses[i].chars, 1, 0, allButtons[nextButton].mouseOver);
				}

				suggestionCount++;
			}
		}
	}
}