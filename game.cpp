#include "game.h"

// Global state definitions
SnakeGameState gameState = { RIGHT, true, false };

Position snake[MAX_LEN];
uint8_t snakeLength = 4; // Start with a snake length of 4

Position fruit[MAX_LEN];
uint8_t fruitSize = 0;

// If the coordinate goes below 0, move it to GRID_SIZE - 1.
// If the coordinate reaches GRID_SIZE, move it back to 0.
// Notes:
//  - We use uint8_t; subtracting 1 from 0 would underflow to 255.
//    To avoid underflow side effects, we check bounds before increment/decrement.

void generateFruitTick() {
    while(rand() < RAND_MAX / 6) {
        fruit[fruitSize++] = Position{rand() % GRID_SIZE, rand() % GRID_SIZE};
    }
}

void ResetGame(void)
{
    // Place snake centered, heading right
    snakeLength = 4;
    uint8_t cx = GRID_SIZE / 2;
    uint8_t cy = GRID_SIZE / 2;
    // Head at [cx, cy], body to the left
    for (uint8_t i = 0; i < snakeLength; ++i) {
        snake[i].x = (uint8_t)(cx - i);
        snake[i].y = cy;
    }
    gameState.currentDirection = RIGHT;
    gameState.isRunning = true;
    gameState.needsReset = false;
}

void moveSnake()
{
    // Shift body so each segment follows the previous one
    for (uint8_t i = snakeLength; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Update head position based on direction with  wrap-around.
    switch (gameState.currentDirection) {
        case UP:
            // If at the top and moving up => wrap to bottom
            if (snake[0].y == 0) {
                snake[0].y = (uint8_t)(GRID_SIZE - 1);
            } else {
                snake[0].y = (uint8_t)(snake[0].y - 1);
            }
            break;
        case DOWN:
            // If at the bottom and moving down => wrap to top
            if (snake[0].y == GRID_SIZE - 1) {
                snake[0].y = 0;
            } else {
                snake[0].y = (uint8_t)(snake[0].y + 1);
            }
            break;
        case LEFT:
            if (snake[0].x == 0) {
                snake[0].x = (uint8_t)(GRID_SIZE - 1);
            } else {
                snake[0].x = (uint8_t)(snake[0].x - 1);
            }
            break;
        case RIGHT:
            if (snake[0].x == GRID_SIZE - 1) {
                snake[0].x = 0;
            } else {
                snake[0].x = (uint8_t)(snake[0].x + 1);
            }
            break;
    }
}

/*************************** SCORING SYSTEM ************************ */
uint8_t score = 0;      // Init. to 0
bool isEaten = 0;

bool fruitEaten(void) {
    // Check with the fruit spawn API 
    return isEaten;
}

void incrScore(bool isEaten, uint8_t score) {
    if (isEaten) score++;
}

void incrLen(bool isEaten, uint8_t snakeLength) {
    if (isEaten && snakeLength < MAX_LEN) {
        snake[snakeLength] = snake[snakeLength - 1];        // Duplicate the tail segment

        snakeLength++;      // Increas length
    }
}
