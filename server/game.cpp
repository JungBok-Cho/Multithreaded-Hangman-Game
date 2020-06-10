#include "game.h"
#include "connection.h"


game::game(const std::string& word) {
    reload(word);
}


void game::reload(const std::string& word) {
    this->word = word;
    this->score = word.length()*1.6;
    this->mask = std::string(word.length(),'_');
}


std::string game::step(char c) {
    std::string rsp;

	std::string newMask = this->mask;
	for (int i = 0; i < (int) word.length(); i++) {
		if (word[i] == c) {
			newMask[i] = c;
		}
	}

	if (newMask == mask) {
		score--;
	} else {
		this->mask = newMask;
	}

    if(word == mask) {
		this->totalScore += this->score;
        rsp = ("type=gameend;score=" + std::to_string(this->score) + ";word=" + this->word
                + ";result=" + "win;totalScore=" + std::to_string(this->totalScore) + ";");
    } else if(score <= 0) {
        rsp = ("type=gameend;score=" + std::to_string(this->score) + ";word=" + this->word
                + ";result=" + "fail;totalScore=" + std::to_string(this->totalScore) + ";");
    } else {
		rsp = ("type=gaming;mask=" + this->mask + ";score=" + std::to_string(this->score) + ";");
    }

    return rsp;
}


std::string game::checkGuess(std::string guess) {
    if(guess == this->word) {
        this->totalScore += this->score;
        return ("type=gameend;score=" + std::to_string(this->score) + ";word=" + this->word
                + ";result=" + "win;totalScore=" + std::to_string(this->totalScore) + ";");
    } else {
        return ("type=gaming;mask=!" + mask + ";score=" + std::to_string(score) + ";");
    }
}

