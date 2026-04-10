#include "stdafx.h"
#include "Control.h"
#include "Guess.h"
#include "Button.h"
#include "Words.h"
#include "Graphics.h"

bool keyPressed(WPARAM key)
{
	if (key == VK_PRIOR) suggestionTopOffset += suggestionSpaceHeight;
	else if (key == VK_NEXT) suggestionTopOffset -= suggestionSpaceHeight;
	else return false;

	return true;
}

bool charTyped(WPARAM ch)
{
	WCHAR input = toupper(ch);
	if (input >= 'A' && input <= 'Z' && currentLetter < 5 && currentLine < MAX_GUESSES)
	{
		guesses[currentLine].letters[currentLetter].ch = input;
		guesses[currentLine].letters[currentLetter].state = LS_UNKNOWN;
		currentLetter++;
		if (currentLetter == 5)
		{
			for (int i = 0; i < MAX_BUTTONS; i++)
			{
				if (allButtons[i].type == BT_CLEAR_LINE && allButtons[i].data1 == currentLine)
				{
					allButtons[i].data2 = 1;
					break;
				}
			}
		}
	}
	else if (input == ' ')
	{
		includeGuesses = !includeGuesses;
		if (includeGuesses) updatePossibleGuesses();
	}
	else if (input == '\b' && currentLetter > 0)
	{
		guesses[currentLine].letters[currentLetter - 1].ch = 0;
		guesses[currentLine].letters[currentLetter - 1].state = LS_EMPTY;
		if (currentLetter == 5)
		{
			for (int i = 0; i < MAX_BUTTONS; i++)
			{
				if (allButtons[i].type == BT_CLEAR_LINE && allButtons[i].data1 == currentLine)
				{
					allButtons[i].data2 = 0;
					break;
				}
			}
		}
		currentLetter--;
	}
	else if (input == '\r' && currentLetter == 5)
	{
		if (checkColours() && (inList(guesses[currentLine], possibleGuesses, totalPossibleGuesses) || inList(guesses[currentLine], possibleAnswers, totalPossibleAnswers)))
		{
			bool moveOn;

			int minInWord[5] = { 0 };
			int numInGuess[5] = { 0 };

			for (int let = 0; let < 5; let++)
			{
				Letter* current = &guesses[currentLine].letters[let];
				current->state = LS_INCORRECT;
				if (currentLine == 0) continue;

				moveOn = false;

				bool occupied[5] = { false };
				bool sameCorrect[5] = { false };
				bool possiblyOccupied[5] = { false };
				int minSameInWord = 0;

				for (int prevLine = 0; prevLine < currentLine; prevLine++)
				{
					Letter* sameLetterSameSlot = &guesses[prevLine].letters[let];
					bool previouslyIncorrect = false;
					bool occursInWord = false;

					if (sameLetterSameSlot->ch == current->ch)
					{
						if (sameLetterSameSlot->state == LS_CORRECT)
						{
							current->state = LS_CORRECT;
							moveOn = true;
							break;
						}
					}

					int misplacedCount = 0;
					int correctCount = 0;

					for (int prevLet = 0; prevLet < 5; prevLet++)
					{
						Letter* l = &guesses[prevLine].letters[prevLet];

						if (l->ch == current->ch)
						{
							switch (l->state)
							{
							case LS_INCORRECT:
								occupied[prevLet] = true;
								previouslyIncorrect = true;
								break;
							case LS_MISPLACED:
								occupied[prevLet] = true;
								occursInWord = true;
								misplacedCount++;
								break;
							case LS_CORRECT:
								sameCorrect[prevLet] = true;
								occursInWord = true;
								correctCount++;
								break;
							}
						}
						else if (l->state == LS_CORRECT) occupied[prevLet] = true;
					}

					minSameInWord = max(minSameInWord, misplacedCount + correctCount);

					if (previouslyIncorrect && !occursInWord)
					{
						moveOn = true;
						break;
					}

					if ((correctCount == 2 && misplacedCount == 1) || (correctCount == 1 && misplacedCount == 2))
					{
						for (int i = 0; i < 5; i++)
						{
							if (sameCorrect[i]) occupied[i] = true;
						}
					}
					else if (correctCount == 1 && misplacedCount == 1)
					{
						for (int i = 0; i < 5; i++)
						{
							if (sameCorrect[i]) possiblyOccupied[i] = true;
						}
					}
				}

				if (!moveOn)
				{
					int sameCorrectCount = 0;

					for (int i = 0; i < 5; i++)
					{
						if (sameCorrect[i]) sameCorrectCount++;
					}

					if (sameCorrectCount == 1)
					{
						for (int i = 0; i < 5; i++)
						{
							if (possiblyOccupied[i]) occupied[i] = true;
						}
					}

					int availableCount = 0;

					for (int i = 0; i < 5; i++)
					{
						if (!occupied[i]) availableCount++;
					}

					if (minSameInWord > 0 && minSameInWord - sameCorrectCount == availableCount) current->state = LS_CORRECT;
				}

				for (int i = 0; i < currentLine; i++)
				{
					int occurrencesInGuess = 0;

					for (int j = 0; j < 5; j++)
					{
						Letter* l = &guesses[i].letters[j];

						if (l->ch == current->ch && l->state != LS_INCORRECT) occurrencesInGuess++;
					}

					minInWord[let] = max(minInWord[let], occurrencesInGuess);
				}

				for (int i = 0; i < 5; i++)
				{
					Letter* l = &guesses[currentLine].letters[i];

					if (l->ch == current->ch && (l->state == LS_MISPLACED || l->state == LS_CORRECT)) numInGuess[let]++;
				}
			}

			int changedToMisplaced[5] = { 0 };

			for (int let = 0; let < 5; let++)
			{
				Letter* current = &guesses[currentLine].letters[let];

				if (current->state == LS_INCORRECT && changedToMisplaced[let] < minInWord[let] - numInGuess[let])
				{
					current->state = LS_MISPLACED;

					for (int i = 0; i < 5; i++)
					{
						if (guesses[currentLine].letters[i].ch == current->ch) changedToMisplaced[i]++;
					}
				}
			}

			for (int i = 0; i < MAX_BUTTONS; i++)
			{
				if (allButtons[i].type == BT_CLEAR_LINE && allButtons[i].data1 == currentLine)
				{
					allButtons[i].data2 = 2;
					break;
				}
			}

			currentLine++;
			currentLetter = 0;

			updatePossibleAnswers();
		}
	}
	else return false;

	return true;
}

bool checkColours()
{
	bool proceed = true;

	if (currentLine == 1)
	{
		for (int let = 0; let < 5; let++)
		{
			Letter* current = &guesses[currentLine - 1].letters[let];
			switch (current->state)
			{
			case LS_INCORRECT_INVALID:
				current->state = LS_INCORRECT;
				break;
			case LS_MISPLACED_INVALID:
				current->state = LS_MISPLACED;
				break;
			case LS_CORRECT_INVALID:
				current->state = LS_CORRECT;
				break;
			}
		}
	}
	else if (currentLine > 1)
	{
		int minInWord[5] = { 0 };
		int numInGuess[5] = { 0 };
		bool actualNum[5] = { false };

		for (int let = 0; let < 5; let++)
		{
			Letter* current = &guesses[currentLine - 1].letters[let];
			bool valid = true;

			switch (current->state)
			{
			case LS_INCORRECT_INVALID:
				current->state = LS_INCORRECT;
				break;
			case LS_MISPLACED_INVALID:
				current->state = LS_MISPLACED;
				break;
			case LS_CORRECT_INVALID:
				current->state = LS_CORRECT;
				break;
			}

			switch (current->state)
			{
			case LS_INCORRECT:
			{
				bool occupied[5] = { false };
				bool sameCorrect[5] = { false };
				bool possiblyOccupied[5] = { false };
				int minSameInWord = 0;

				for (int prevLine = 0; prevLine < currentLine - 1; prevLine++)
				{
					Letter* sameLetterSameSlot = &guesses[prevLine].letters[let];
					if (sameLetterSameSlot->ch == current->ch && sameLetterSameSlot->state == LS_CORRECT)
					{
						valid = false;
						break;
					}

					int correctCount = 0;
					int misplacedCount = 0;

					for (int prevLet = 0; prevLet < 5; prevLet++)
					{
						Letter* l = &guesses[prevLine].letters[prevLet];

						if (l->ch == current->ch)
						{
							switch (l->state)
							{
							case LS_INCORRECT:
								occupied[prevLet] = true;
								break;
							case LS_MISPLACED:
								occupied[prevLet] = true;
								misplacedCount++;
								break;
							case LS_CORRECT:
								sameCorrect[prevLet] = true;
								correctCount++;
								break;
							}
						}
						else if (l->state == LS_CORRECT) occupied[prevLet] = true;
					}

					minSameInWord = max(minSameInWord, misplacedCount + correctCount);

					if ((correctCount == 2 && misplacedCount == 1) || (correctCount == 1 && misplacedCount == 2))
					{
						for (int i = 0; i < 5; i++)
						{
							if (sameCorrect[i]) occupied[i] = true;
						}
					}
					else if (correctCount == 1 && misplacedCount == 1)
					{
						for (int i = 0; i < 5; i++)
						{
							if (sameCorrect[i]) possiblyOccupied[i] = true;
						}
					}
				}

				if (valid)
				{
					int sameCorrectCount = 0;

					for (int i = 0; i < 5; i++)
					{
						if (sameCorrect[i]) sameCorrectCount++;
					}

					if (sameCorrectCount == 1)
					{
						for (int i = 0; i < 5; i++)
						{
							if (possiblyOccupied[i]) occupied[i] = true;
						}
					}

					int availableCount = 0;

					for (int i = 0; i < 5; i++)
					{
						if (!occupied[i]) availableCount++;
					}

					if (minSameInWord > 0 && minSameInWord - sameCorrectCount == availableCount) valid = false;
				}

				if (!valid)
				{
					proceed = false;
					current->state = LS_INCORRECT_INVALID;
				}

				break;
			}
			case LS_MISPLACED:
			{
				bool occupied[5] = { false };
				bool sameCorrect[5] = { false };
				bool possiblyOccupied[5] = { false };
				int minSameInWord = 0;

				for (int prevLine = 0; prevLine < currentLine - 1; prevLine++)
				{
					Letter* sameLetterSameSlot = &guesses[prevLine].letters[let];
					bool previouslyIncorrect = false;
					bool occursInWord = false;

					if (sameLetterSameSlot->ch == current->ch)
					{
						if (sameLetterSameSlot->state == LS_CORRECT)
						{
							valid = false;
							break;
						}
						
						if (sameLetterSameSlot->state == LS_INCORRECT) previouslyIncorrect = true;
					}

					int correctCount = 0;
					int misplacedCount = 0;

					for (int prevLet = 0; prevLet < 5; prevLet++)
					{
						Letter* l = &guesses[prevLine].letters[prevLet];

						if (l->ch == current->ch)
						{
							switch (l->state)
							{
							case LS_INCORRECT:
								occupied[prevLet] = true;
								break;
							case LS_MISPLACED:
								occupied[prevLet] = true;
								occursInWord = true;
								misplacedCount++;
								break;
							case LS_CORRECT:
								sameCorrect[prevLet] = true;
								occursInWord = true;
								correctCount++;
								break;
							}
						}
						else if (l->state == LS_CORRECT) occupied[prevLet] = true;
					}

					minSameInWord = max(minSameInWord, misplacedCount + correctCount);

					if (previouslyIncorrect && !occursInWord)
					{
						valid = false;
						break;
					}

					if ((correctCount == 2 && misplacedCount == 1) || (correctCount == 1 && misplacedCount == 2))
					{
						for (int i = 0; i < 5; i++)
						{
							if (sameCorrect[i]) occupied[i] = true;
						}
					}
					else if (correctCount == 1 && misplacedCount == 1)
					{
						for (int i = 0; i < 5; i++)
						{
							if (sameCorrect[i]) possiblyOccupied[i] = true;
						}
					}
				}

				if (valid)
				{
					int sameCorrectCount = 0;

					for (int i = 0; i < 5; i++)
					{
						if (sameCorrect[i]) sameCorrectCount++;
					}

					if (sameCorrectCount == 1)
					{
						for (int i = 0; i < 5; i++)
						{
							if (possiblyOccupied[i]) occupied[i] = true;
						}
					}

					int availableCount = 0;

					for (int i = 0; i < 5; i++)
					{
						if (!occupied[i]) availableCount++;
					}

					if (minSameInWord > 0 && minSameInWord - sameCorrectCount == availableCount) valid = false;
				}

				if (!valid)
				{
					proceed = false;
					current->state = LS_MISPLACED_INVALID;
				}

				break;
			}
			case LS_CORRECT:
			{
				bool skipChecks = false;
				bool correct[5] = { false };
				bool totalCorrectKnown = false;
				int totalCorrect = 0;

				for (int prevLine = 0; prevLine < currentLine - 1; prevLine++)
				{
					Letter* sameLetterSameSlot = &guesses[prevLine].letters[let];
					if (sameLetterSameSlot->ch == current->ch)
					{
						if (sameLetterSameSlot->state == LS_CORRECT)
						{
							skipChecks = true;
							break;
						}
						else if (sameLetterSameSlot->state == LS_MISPLACED)
						{
							valid = false;
							break;
						}
					}
					else
					{
						if (sameLetterSameSlot->state == LS_CORRECT)
						{
							valid = false;
							break;
						}
					}

					bool incorrect = false;
					int misplacedCount = 0;
					int correctCount = 0;

					for (int prevLet = 0; prevLet < 5; prevLet++)
					{
						Letter* l = &guesses[prevLine].letters[prevLet];

						if (l->ch == current->ch)
						{
							if (l->state == LS_INCORRECT) incorrect = true;
							else if (l->state == LS_MISPLACED) misplacedCount++;
							else if (l->state == LS_CORRECT)
							{
								correct[prevLet] = true;
								correctCount++;
							}
						}
					}

					if (incorrect)
					{
						if (misplacedCount > 0)
						{
							totalCorrectKnown = true;
							totalCorrect = misplacedCount + correctCount;
						}
						else
						{
							valid = false;
							break;
						}
					}
				}

				if (!skipChecks && valid)
				{
					int uniqueCorrect = 0;

					for (int i = 0; i < 5; i++)
					{
						if (correct[i]) uniqueCorrect++;
					}

					if (totalCorrectKnown && uniqueCorrect == totalCorrect) valid = false;
				}

				if (!skipChecks && valid)
				{
					for (int c = 0; c < 26; c++)
					{
						if (c == current->ch - 'A') continue;

						Letter testLet;
						testLet.ch = 'A' + c;
						testLet.zero = 0;
						testLet.state = LS_INCORRECT;

						bool notInWord = false;
						bool inWord = false;
						bool occupied[5] = { false };
						bool sameCorrect[5] = { false };
						bool possiblyOccupied[5] = { false };
						int minSameInWord = 0;

						for (int prevLine = 0; prevLine < currentLine - 1; prevLine++)
						{
							int correctCount = 0;
							int misplacedCount = 0;
							bool incorrectOccurrence = false;

							for (int prevLet = 0; prevLet < 5; prevLet++)
							{
								Letter* l = &guesses[prevLine].letters[prevLet];

								if (l->ch == testLet.ch)
								{
									if (l->state == LS_INCORRECT)
									{
										incorrectOccurrence = true;
										break;
									}
									if (l->state == LS_MISPLACED)
									{
										occupied[prevLet] = true;
										inWord = true;
										misplacedCount++;
									}
									else if (l->state == LS_CORRECT)
									{
										sameCorrect[prevLet] = true;
										inWord = true;
										correctCount++;
									}
								}
								else if (l->state == LS_CORRECT) occupied[prevLet] = true;
							}

							minSameInWord = max(minSameInWord, misplacedCount + correctCount);

							if (incorrectOccurrence && misplacedCount == 0 && correctCount == 0)
							{
								notInWord = true;
								break;
							}

							if ((correctCount == 2 && misplacedCount == 1) || (correctCount == 1 && misplacedCount == 2))
							{
								for (int i = 0; i < 5; i++)
								{
									if (sameCorrect[i]) occupied[i] = true;
								}
							}
							else if (correctCount == 1 && misplacedCount == 1)
							{
								for (int i = 0; i < 5; i++)
								{
									if (sameCorrect[i]) possiblyOccupied[i] = true;
								}
							}
						}

						if (!inWord || notInWord) continue;

						int sameCorrectCount = 0;

						for (int i = 0; i < 5; i++)
						{
							if (sameCorrect[i]) sameCorrectCount++;
						}

						if (sameCorrectCount == 1)
						{
							for (int i = 0; i < 5; i++)
							{
								if (possiblyOccupied[i]) occupied[i] = true;
							}
						}

						int availableCount = 0;

						for (int i = 0; i < 5; i++)
						{
							if (!occupied[i]) availableCount++;
						}

						if (minSameInWord > 0 && minSameInWord - sameCorrectCount == availableCount && !occupied[let])
						{
							valid = false;
							break;
						}
					}
				}

				if (!valid)
				{
					proceed = false;
					current->state = LS_CORRECT_INVALID;
				}

				break;
			}
			}

			for (int i = 0; i < currentLine - 1; i++)
			{
				int occurrencesInGuess = 0;

				for (int j = 0; j < 5; j++)
				{
					Letter* l = &guesses[i].letters[j];

					if (l->ch == current->ch)
					{
						if (l->state == LS_INCORRECT) actualNum[let] = true;
						else occurrencesInGuess++;
					}
				}

				minInWord[let] = max(minInWord[let], occurrencesInGuess);
			}

			for (int i = 0; i < 5; i++)
			{
				Letter* l = &guesses[currentLine - 1].letters[i];

				if (l->ch == current->ch && (l->state == LS_MISPLACED || l->state == LS_CORRECT)) numInGuess[let]++;
			}
		}

		for (int let = 0; let < 5; let++)
		{
			Letter* current = &guesses[currentLine - 1].letters[let];

			if (current->state == LS_INCORRECT)
			{
				if (numInGuess[let] < minInWord[let])
				{
					proceed = false;
					current->state = LS_INCORRECT_INVALID;
				}
			}
			else
			{
				if (actualNum[let] && numInGuess[let] > minInWord[let])
				{
					proceed = false;
					if (current->state == LS_MISPLACED) current->state = LS_MISPLACED_INVALID;
					else current->state = LS_CORRECT_INVALID;
				}
			}
		}
	}

	if (currentLine != 0)
	{
		for (int let = 0; let < 5; let++)
		{
			Letter* current = &guesses[currentLine - 1].letters[let];

			int occurrences = guesses[currentLine - 1].countOccurrences(current->ch);

			if (occurrences >= 3)
			{
				bool misplaced[5] = { false };
				int misplacedCount = 0;

				for (int i = 0; i < 5; i++)
				{
					Letter* l = &guesses[currentLine - 1].letters[i];
					if (l->ch == current->ch && (l->state == LS_MISPLACED || l->state == LS_MISPLACED_INVALID))
					{
						misplaced[i] = true;
						misplacedCount++;
					}
				}

				if (misplacedCount >= 3)
				{
					for (int i = 0; i < 5; i++)
					{
						if (misplaced[i])
						{
							guesses[currentLine - 1].letters[i].state = LS_MISPLACED_INVALID;
						}
					}

					break;
				}
			}
		}
	}

	return proceed;
}

void updatePossibleAnswers()
{
	for (int i = 0; i < totalPossibleAnswers; i++) possibleAnswers[i].impossible = 0;
	numPossibleAnswers = totalPossibleAnswers;

	for (int guess = 0; guess < currentLine; guess++)
	{
		Guess* g = &guesses[guess];
		bool correct[5] = { false };
		int minInWord[5] = { 0 };
		bool minIsNum[5] = { false };

		for (int let = 0; let < 5; let++)
		{
			Letter* l = &g->letters[let];

			if (l->state == LS_CORRECT) correct[let] = true;

			if (l->state == LS_INCORRECT)
			{
				minIsNum[let] = true;
			}
			else
			{
				for (int i = 0; i < 5; i++)
				{
					if (g->letters[i].ch == l->ch)
					{
						minInWord[i]++;
						if (minIsNum[let]) minIsNum[i] = true;
					}
				}
			}
		}

		for (int word = 0; word < totalPossibleAnswers; word++)
		{
			Word* w = possibleAnswers + word;

			if (!w->impossible)
			{
				for (int i = 0; i < 5; i++)
				{
					if (correct[i] && g->letters[i].ch != w->chars[i] || !correct[i] && g->letters[i].ch == w->chars[i])
					{
						w->impossible = 1;
						break;
					}
				}
			}

			if (!w->impossible)
			{
				for (int i = 0; i < 5; i++)
				{
					int count = w->countOccurrences(g->letters[i].ch);

					if (count < minInWord[i] || minIsNum[i] && count != minInWord[i])
					{
						w->impossible = 1;
						break;
					}
				}
			}
		}
	}

	for (int word = 0; word < totalPossibleAnswers; word++)
	{
		if ((possibleAnswers + word)->impossible) numPossibleAnswers--;
	}

	if (includeGuesses) updatePossibleGuesses();
}

void updatePossibleGuesses()
{
	for (int i = 0; i < totalPossibleGuesses; i++) possibleGuesses[i].impossible = 0;
	numPossibleGuesses = totalPossibleGuesses;

	for (int guess = 0; guess < currentLine; guess++)
	{
		Guess* g = &guesses[guess];
		bool correct[5] = { false };
		int minInWord[5] = { 0 };
		bool minIsNum[5] = { false };

		for (int let = 0; let < 5; let++)
		{
			Letter* l = &g->letters[let];

			if (l->state == LS_CORRECT) correct[let] = true;

			if (l->state == LS_INCORRECT)
			{
				minIsNum[let] = true;
			}
			else
			{
				for (int i = 0; i < 5; i++)
				{
					if (g->letters[i].ch == l->ch)
					{
						minInWord[i]++;
						if (minIsNum[let]) minIsNum[i] = true;
					}
				}
			}
		}

		for (int word = 0; word < totalPossibleGuesses; word++)
		{
			Word* w = possibleGuesses + word;

			if (!w->impossible)
			{
				for (int i = 0; i < 5; i++)
				{
					if (correct[i] && g->letters[i].ch != w->chars[i] || !correct[i] && g->letters[i].ch == w->chars[i])
					{
						w->impossible = 1;
						break;
					}
				}
			}

			if (!w->impossible)
			{
				for (int i = 0; i < 5; i++)
				{
					int count = w->countOccurrences(g->letters[i].ch);

					if (count < minInWord[i] || minIsNum[i] && count != minInWord[i])
					{
						w->impossible = 1;
						break;
					}
				}
			}
		}
	}

	for (int word = 0; word < totalPossibleGuesses; word++)
	{
		if ((possibleGuesses + word)->impossible) numPossibleGuesses--;
	}
}