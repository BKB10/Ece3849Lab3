// Minimal Snake (FreeRTOS + GRLIB + Mutex)
// Hardware: TM4C1294XL + Crystalfontz 128x128 LCD

#include <stdint.h>
#include <stdbool.h>

extern "C" {
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "sysctl_pll.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "buzzer.h"
}

// Board drivers (provided in project includes)
#include "button.h"
#include "joystick.h"


//extern QueueHandle_t xBuzzerQueue;

// App modules per lab structure
#include "app_objects.h"
#include "game.h"
#include "display.h"

// Shared objects
tContext gContext;
uint32_t gSysClk;

// Buttons used for pause/reset
static Button btnPause(S1);
static Button btnReset(S2);
// Joystick (axes + stick push). Pins from HAL pins.h (BOOSTERPACK1)
static Joystick gJoystick(JSX, JSY, JS1);

SemaphoreHandle_t fruitMutex;

// Config
#define INPUT_TICK_MS   10U
#define FRUIT_TICK_MS   250U

static QueueHandle_t gBuzzerQueue;

// Prototypes
static void configureSystemClock(void);
static void vFruitTask(void* args);
static void vInputTask(void *pvParameters);
static void vSnakeTask(void *pvParameters);
static void vRenderTask(void *pvParameters);
static void vBuzzerTask(void* pvParameters);

void Buzzer_Post(uint16_t frequency, uint16_t durationMS) {
    BuzzerEvent event = {frequency, durationMS};

    xQueueSend(gBuzzerQueue, &event, 0);
}

int main(void)
{
    IntMasterDisable();
    FPUEnable();
    FPULazyStackingEnable();

    configureSystemClock();

    fruitMutex = xSemaphoreCreateMutex();
    if(!fruitMutex) {
        while(1);
    }


    // Init buttons and joystick
    btnPause.begin();
    btnReset.begin();
    gJoystick.begin();
    btnPause.setTickIntervalMs(INPUT_TICK_MS);
    btnReset.setTickIntervalMs(INPUT_TICK_MS);
    gJoystick.setTickIntervalMs(INPUT_TICK_MS);
    btnPause.setDebounceMs(30);
    btnReset.setDebounceMs(30);
    // Optional joystick tuning
    gJoystick.setDeadzone(0.15f);

    Buzzer_Init();      // Init buzzer

    IntMasterEnable();

    // Create tasks (priorities per lab suggestion)
    xTaskCreate(vFruitTask, "Fruit", 512, NULL, 1, NULL);
    xTaskCreate(vInputTask,  "Input",  512, NULL, 2, NULL);
    xTaskCreate(vSnakeTask,  "Snake",  512, NULL, 3, NULL);
    xTaskCreate(vRenderTask, "Render", 1024, NULL, 1, NULL);
    xTaskCreate(vBuzzerTask, "Buzzer", 512, NULL, 1, NULL);

    gBuzzerQueue = xQueueCreate(4, sizeof(BuzzerEvent));

    srand(gSysClk * 1000000);

    vTaskStartScheduler();

    while (1);
}

static void configureSystemClock(void)
{
    gSysClk = SysCtlClockFreqSet(
        SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
        SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
        120000000);
}

static void vFruitTask(void *args) {
    TickType_t lastWakeTime = xTaskGetTickCount();

    while(1) {
        generateFruitTick();

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(FRUIT_TICK_MS));
    }
}

// Reads joystick/buttons and updates gameState
static void vInputTask(void *pvParameters)
{
    (void)pvParameters;
    for (;;) {
        // Hardware button + joystick polling
        btnPause.tick();
        btnReset.tick();
        gJoystick.tick();

        // Toggle pause on S1
        if (btnPause.wasPressed()) {
            gameState.isRunning = !gameState.isRunning;
            Buzzer_Post(440, 250);      // Play sound
        }
        // Request reset on S2
        if (btnReset.wasPressed()) {
            gameState.needsReset = true;
            Buzzer_Post(850, 250);     // Play sound
        }

        // Joystick 8-way direction mapping to game directions
        int8_t testPos;
        switch (gJoystick.direction8()) {
            case JoystickDir::N:
            case JoystickDir::NE:
            case JoystickDir::NW:
                testPos = snake[0].y - 1;
                if(testPos < 0) {
                    testPos = GRID_SIZE - 1;
                } else if(testPos > GRID_SIZE - 1) {
                    testPos = 0;
                }
                if(!positionHasSnake(snake[0].x, testPos)) {
                    gameState.currentDirection = UP;
                }
                break;
            case JoystickDir::S:
            case JoystickDir::SE:
            case JoystickDir::SW:
                testPos = snake[0].y + 1;
                if(testPos < 0) {
                    testPos = GRID_SIZE - 1;
                } else if(testPos > GRID_SIZE - 1) {
                    testPos = 0;
                }
                if(!positionHasSnake(snake[0].x, testPos)) {
                    gameState.currentDirection = DOWN;
                }
                break;
            case JoystickDir::E:
                testPos = snake[0].x + 1;
                if(testPos < 0) {
                    testPos = GRID_SIZE - 1;
                } else if(testPos > GRID_SIZE - 1) {
                    testPos = 0;
                }
                if(!positionHasSnake(testPos, snake[0].y)) {
                    gameState.currentDirection = RIGHT;
                }
                break;
            case JoystickDir::W:
                testPos = snake[0].x - 1;
                if(testPos < 0) {
                    testPos = GRID_SIZE - 1;
                } else if(testPos > GRID_SIZE - 1) {
                    testPos = 0;
                }
                if(!positionHasSnake(testPos, snake[0].y)) {
                    gameState.currentDirection = LEFT;
                }
                break;
            case JoystickDir::Center:
            default:
                // keep last direction
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(INPUT_TICK_MS));
    }
}

// Advances the snake periodically
static void vSnakeTask(void *pvParameters)
{
    (void)pvParameters;
    ResetGame();
    for(;;){
        if (gameState.needsReset) {
            ResetGame();
        }
        if (gameState.isRunning) {
            moveSnake();
        }
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

// Renders current frame to LCD (guarded by mutex)
static void vRenderTask(void *pvParameters)
{
    (void)pvParameters;
    LCD_Init();
    TickType_t last = xTaskGetTickCount();
    for(;;)
    {
        //xSemaphoreTake(xMutexLcd, portMAX_DELAY);
        DrawGame(&gameState);
        //xSemaphoreGive(xMutexLcd);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(33));
    }
}

// Buzzer task to handle buzzer events posted by other tasks
static void vBuzzerTask(void* pvParameters)
{
    BuzzerEvent event;

    for(;;)
    {
        if(xQueueReceive(gBuzzerQueue, &event, portMAX_DELAY) == pdTRUE)
        {
            buzzerStart(event.frequency);
            vTaskDelay(pdMS_TO_TICKS(event.duration));
            buzzerStop();
        }
    }
}
