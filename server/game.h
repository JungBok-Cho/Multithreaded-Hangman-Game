#ifndef INC_5042_GAME_H
#define INC_5042_GAME_H
#pragma once
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <string>

class connection;

class game {
public:
	/**
	 * This function creates a game object to guess a word
	 *
	 * INPUT: string a word to be guessed
	 * OUTPUT: game object for guessing the supplied word
	*/
    game(const std::string& word);

	/**
	 * This function re-initializes the game object if player chooses
	 * to play again after winning or losing
	 *
	 * INPUT: string a word to be guessed
	 * OUTPUT: game object modified from current state, back to initial state
	*/
    void reload(const std::string& word);

	/**
	 * This function is 1 of 2 for the main game loop. 
	 * it steps through each char in the string and compares it to the mask
	 *
	 * INPUT: char character guessed
	 * OUTPUT: RCP to be sent to player with info about the state of the game 
	 * after their guess
	*/
    std::string step(char c);

	/**
	 * This function is the second main game loop.
	 * it checks to see if the player correctly guessed the word 
	 *
	 * INPUT: string word guessed.
	 * OUTPUT: RCP to be sent to player with info about the state of the game
	 * after their guess.
	*/
    std::string checkGuess(std::string);

	// Getter for the score
    inline int getScore() {
        return score;
    }

    // Getter for the mask
    inline std::string& getMask() {
        return mask;
    }

    // Getter for the word
    inline std::string& getWord() {
        return word;
    }

    // Getter for the total score
    inline int getTotalScore() {
        return totalScore;
    }

private:
	///Private variables

	//The current score for the current round.
    int score;

	//The total score for the players session
	int totalScore = 0;

	//the word to be guessed
    std::string word;

	/* the mask the player is shown which displays the total number
	   of letters in the word, as well as which letters the player
	   has correctly guessed.
	*/
    std::string mask;
};

#endif //INC_5042_GAME_H
