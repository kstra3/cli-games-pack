#ifndef GAMES_H
#define GAMES_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

// Function prototypes for all games
void play_rock_paper_scissors(void);
void play_guess_number(void);
void play_tic_tac_toe(void);
void play_hangman(void);
void play_word_scramble(void);
void play_coin_flip(void);
void play_blackjack(void);
void play_bulls_and_cows(void);
void play_ascii_racing(void);

// Utility functions
void clear_input_buffer(void);
void pause_and_continue(void);

#endif // GAMES_H
