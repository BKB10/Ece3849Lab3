#include "game.h"

extern "C" {
#include "buzzer.h"
}

// Global state definitions
SnakeGameState gameState = { RIGHT, true, false, false };

Position snake[MAX_LEN];
uint8_t snakeLength = 4; // Start with a snake length of 4

Position fruit[MAX_LEN];
uint8_t fruitSize = 0;

uint8_t score = 0;

// If the coordinate goes below 0, move it to GRID_SIZE - 1.
// If the coordinate reaches GRID_SIZE, move it back to 0.
// Notes:
//  - We use uint8_t; subtracting 1 from 0 would underflow to 255.
//    To avoid underflow side effects, we check bounds before increment/decrement.

extern void Buzzer_Post(uint16_t, uint16_t);

uint8_t generatedX;
uint8_t generatedY;

bool positionHasSnake(uint8_t x, uint8_t y) {
    for(int i = 0; i < snakeLength; i ++) {
        if(snake[i].x == x && snake[i].y == y) {
            return true;
        }
    }

    return false;
}

void generateFruitTick() {
    generatedX = rand() % GRID_SIZE;
    generatedY = rand() % GRID_SIZE;
    while(gameState.isRunning && rand() < RAND_MAX / 10) {
        if(!positionHasSnake(generatedX, generatedY)) {
            xSemaphoreTake(fruitMutex, portMAX_DELAY);
            fruit[fruitSize++] = Position{generatedX, generatedY};
            xSemaphoreGive(fruitMutex);
        }
    }
}

void ResetGame()
{
    // Place snake centered, heading right
    fruitSize = 0;
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
    gameState.lose = false;

    score = 0;
}

void eatFruit(uint8_t fruitIndex) {
    snakeLength++;
    score++;
    Buzzer_Post(1500, 100); // Play a short beep on eating fruit

    xSemaphoreTake(fruitMutex, portMAX_DELAY);
    fruitSize --;
    //Shift down
    for(int i = fruitIndex; i < fruitSize; i ++) {
        fruit[i] = fruit[i + 1];
    }
    xSemaphoreGive(fruitMutex);
}

bool isColliding() {
    for(int i = 1; i < snakeLength; i ++) {
        if(snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            return true;
        }
    }

    return false;
}

void moveSnake()
{
    if(gameState.isRunning && !gameState.lose) {
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
                    snake[0].y -= 1;
                }
                break;
            case DOWN:
                // If at the bottom and moving down => wrap to top
                if (snake[0].y == GRID_SIZE - 1) {
                    snake[0].y = 0;
                } else {
                    snake[0].y += 1;
                }
                break;
            case LEFT:
                if (snake[0].x == 0) {
                    snake[0].x = (uint8_t)(GRID_SIZE - 1);
                } else {
                    snake[0].x -= 1;
                }
                break;
            case RIGHT:
                if (snake[0].x == GRID_SIZE - 1) {
                    snake[0].x = 0;
                } else {
                    snake[0].x += 1;
                }
                break;
        }

        if(isColliding()) {
            gameState.lose = true;
            Buzzer_Post(3000, 150);
            Buzzer_Post(2500, 150);
            Buzzer_Post(3000, 150);
            Buzzer_Post(2500, 150);
        } else {
            //Check if head is covering fruit
            for(int i = 0; i < fruitSize; i ++) {
                if(snake[0].x == fruit[i].x && snake[0].y == fruit[i].y) {
                    eatFruit(i);
                }
            }
        }
    }
}
