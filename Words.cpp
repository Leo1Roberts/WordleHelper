#include "stdafx.h"
#include "stdio.h"
#include "Words.h"
#include "Guess.h"

Word* possibleGuesses;
int totalPossibleGuesses;
int numPossibleGuesses;
Word* possibleAnswers;
int totalPossibleAnswers;
int numPossibleAnswers;
int includeGuesses;

int Word::countOccurrences(char letter)
{
	int count = 0;

	for (int i = 0; i < 5; i++) if (chars[i] == letter) count++;

	return count;
}

bool init_words()
{
	bool success = loadWords("guesses.txt", &possibleGuesses, &totalPossibleGuesses) && loadWords("answers.txt", &possibleAnswers, &totalPossibleAnswers);
	numPossibleGuesses = totalPossibleGuesses;
	numPossibleAnswers = totalPossibleAnswers;
	return success;
}

void delete_words()
{
	delete possibleGuesses;
	delete possibleAnswers;
}

bool loadWords(char* fileName, Word** arr, int* totalWords)
{
	FILE* fin = fopen(fileName, "rt");

	if (fin == NULL) return false;

	*totalWords = countWords(fin);

	if (*totalWords == 0)
	{
		fclose(fin);
		return false;
	}

	Word* alloc = new Word[*totalWords];
	memset(alloc, 0, *totalWords * sizeof(Word));

	fseek(fin, 0, 0);

	fillWords(fin, alloc, *totalWords);
	
	fclose(fin);

	*arr = alloc;

	return true;
}

int countWords(FILE* file) // Returns 0 if there is an error
{
	int c = toupper(fgetc(file));
	int letterCount = 0;
	int wordCount = 0;

	while (c != EOF)
	{
		if (c >= 'A' && c <= 'Z') letterCount++;
		else if (c == '\n')
		{
			if (letterCount == 5)
			{
				wordCount++;
				letterCount = 0;
			}
			else if (letterCount != 5) return 0; // Word wrong length (ignore blank lines)
		}
		else return 0; // Invalid character

		c = toupper(fgetc(file));
	}

	if (letterCount != 0 && letterCount != 5) return 0; // Last word is wrong length

	return wordCount;
}

void fillWords(FILE* file, Word* arr, int totalWords)
{
	int c = toupper(fgetc(file));
	int letter = 0;
	int word = 0;

	while (word < totalWords)
	{
		if (c == '\n' && letter == 5 || c == EOF)
		{
			word++;
			letter = 0;
		}
		else
		{
			arr[word].chars[letter] = c;
			letter++;
		}

		c = toupper(fgetc(file));
	}
}

bool inList(Guess guess, Word* list, int totalWords)
{
	bool found = false;

	for (int word = 0; word < totalWords; word++)
	{
		found = true;

		for (int let = 0; let < 5; let++)
		{
			if (guess.letters[let].ch != list[word].chars[let])
			{
				found = false;
				break;
			}
		}

		if (found == false) continue;

		break;
	}

	return found;
}