#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_TRIES 6
#define MAX_WORD_LENGTH 20

// Function prototypes
void clearInputBuffer();
void displayGame(int tries, const char *currentWord, const char *guessedLetters);
int checkWin(const char *currentWord, const char *guessedLetters);
int alreadyGuessed(char letter, const char *guessedLetters);
void updateGuessedLetters(char letter, char *guessedLetters);
int chooseWord(char *wordBuffer);

int main() {
    char word[MAX_WORD_LENGTH];
    char guessedLetters[26] = {0}; // Array to track guessed letters (a-z)
    int tries = 0;

    srand(time(NULL)); // Initialize random seed

    if (chooseWord(word) == 0) {
        printf("Failed to read word list. Exiting...\n");
        return 1;
    }

    printf("Welcome to Hangman!\n");

    while (tries < MAX_TRIES) {
        displayGame(tries, word, guessedLetters);

        // Get user input
        char guess;
        printf("Enter a letter guess: ");
        scanf(" %c", &guess);
        clearInputBuffer();

        // Check if the letter has already been guessed
        if (alreadyGuessed(guess, guessedLetters)) {
            printf("You've already guessed '%c'. Please try again.\n", guess);
            continue;
        }

        // Update guessed letters
        updateGuessedLetters(guess, guessedLetters);

        // Check if the guess is correct
        if (strchr(word, guess) == NULL) {
            tries++;
            printf("Incorrect guess!\n");
        } else {
            printf("Correct guess!\n");
        }

        // Check if the player has won
        if (checkWin(word, guessedLetters)) {
            displayGame(tries, word, guessedLetters);
            printf("Congratulations! You won!\n");
            break;
        }
    }

    // If out of tries, display the word and end game
    if (tries >= MAX_TRIES) {
        printf("Out of tries! The word was: %s\n", word);
        printf("Game over.\n");
    }

    return 0;
}

// Function to clear input buffer
void clearInputBuffer() {
    while (getchar() != '\n');
}

// Function to display the current state of the game
void displayGame(int tries, const char *currentWord, const char *guessedLetters) {
    printf("\nCurrent word: ");
    for (int i = 0; i < strlen(currentWord); i++) {
        if (strchr(guessedLetters, currentWord[i]) != NULL) {
            printf("%c ", currentWord[i]);
        } else {
            printf("_ ");
        }
    }
    printf("\n");

    printf("Guessed letters: ");
    for (char c = 'a'; c <= 'z'; c++) {
        if (strchr(guessedLetters, c) != NULL) {
            printf("%c ", c);
        } else {
            printf("_ ");
        }
    }
    printf("\n");

    printf("Tries left: %d/%d\n", tries, MAX_TRIES);
}

// Function to check if the player has won
int checkWin(const char *currentWord, const char *guessedLetters) {
    for (int i = 0; i < strlen(currentWord); i++) {
        if (strchr(guessedLetters, currentWord[i]) == NULL) {
            return 0; // Not all letters in currentWord have been guessed
        }
    }
    return 1; // All letters in currentWord have been guessed
}

// Function to check if a letter has already been guessed
int alreadyGuessed(char letter, const char *guessedLetters) {
    return strchr(guessedLetters, letter) != NULL;
}

// Function to update guessed letters array
void updateGuessedLetters(char letter, char *guessedLetters) {
    guessedLetters[strlen(guessedLetters)] = letter;
}

// Function to choose a random word from a predefined list
int chooseWord(char *wordBuffer) {
    // List of words to choose from
    const char *wordList[] = {
        "apple",
        "banana",
        "orange",
        "grape",
        "strawberry",
        "pineapple",
        "watermelon"
    };
    int numWords = sizeof(wordList) / sizeof(wordList[0]);

    // Pick a random word from the list
    int index = rand() % numWords;
    strcpy(wordBuffer, wordList[index]);

    return 1;
}
