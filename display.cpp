#include <stdint.h>

extern "C" {
#include "Crystalfontz128x128_ST7735.h"
#include "grlib/grlib.h"
}

#include "app_objects.h"
#include "game.h"

void LCD_Init(void)
{
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
    GrContextInit(&gContext, &g_sCrystalfontz128x128);
    GrContextFontSet(&gContext, &g_sFontFixed6x8);

    tRectangle full = {0, 0, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &full);
#ifdef GrFlush
    GrFlush(&gContext);
#endif
}

static inline void drawCell(uint8_t gx, uint8_t gy, uint32_t color)
{
    // Convert grid cell to pixel rectangle
    int16_t x0 = (int16_t)(gx * CELL_SIZE);
    int16_t y0 = (int16_t)(gy * CELL_SIZE);
    tRectangle r = { x0, y0, (int16_t)(x0 + CELL_SIZE - 1), (int16_t)(y0 + CELL_SIZE - 1) };
    GrContextForegroundSet(&gContext, color);
    GrRectFill(&gContext, &r);
}

void drawFruit() {
    for(uint8_t i = 0; i < fruitSize; i ++) {
        drawCell(fruit[i].x, fruit[i].y, ClrRed);
    }
}

void DrawGame(const SnakeGameState* state)
{
    (void)state; // not used for minimal version yet

    // Clear background
    tRectangle full = {0, 0, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &full);

    // Draw snake
    for (uint8_t i = 0; i < snakeLength; ++i) {
        drawCell(snake[i].x, snake[i].y, i == 0 ? ClrGreen : ClrYellow);
    }

    drawFruit();

#ifdef GrFlush
    GrFlush(&gContext);
#endif
}

/*
* Function to display the score on the LCD
*/
void DrawScore(uint8_t score) {
    char str[10];
    snprint9(str, sizeof(str), "%0u", score);

    GrContextForegroundSet(&gContext, ClrYellow);
    GrStringDrawCentered(&gContext, "STOPWATCH", -1, 64, 15, false);
}

