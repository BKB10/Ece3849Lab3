#pragma once

#include <stdbool.h>
#include <stdint.h>

/********************** SNACK GENERATION AND MOVEMENT ****************** */
#include <stdlib.h>
#include <vector>

// Direction for snake movement
typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

// High-level game state flags
typedef struct SnakeGameState {
    Direction currentDirection;
    bool isRunning;
    bool needsReset;
} SnakeGameState;

extern SnakeGameState gameState;

// Grid configuration (128x128 display, 8x8 cells)
#define GRID_SIZE 16
#define CELL_SIZE 8
#define MAX_LEN   256  // Maximum snake length (16x16 grid)

// Position on grid
typedef struct { uint8_t x, y; } Position;

// Snake representation
extern Position snake[MAX_LEN];
extern uint8_t snakeLength;

// Basic API
void ResetGame(void);
void moveSnake(void);

/****************** SCORE SYSTEM API ************************* */
extern uint8_t score;           // Keep track of score --- MAX_SCORE = 256?
extern bool isEaten;

bool fruitEaten(void);              // Check if fruit is eaten
void incrScore(bool, uint8_t);      // Increase score once the fruit is eaten
void incrLen(bool, uint8_t);        // Increase snake lenght once the fruit is eaten

extern Position fruit[MAX_LEN];
extern uint8_t fruitSize;

void generateFruitTick();

// Basic API
void ResetGame(void);
void moveSnake(void);
