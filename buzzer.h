#pragma once

#include <stdint.h>
#include <stdbool.h>

// Include FreeRTOS queue API for BuzzerEvent queue
#include "FreeRTOS.h"
#include "queue.h"

// Structure to define a buzzer sound event
typedef struct {
    uint16_t frequency;   // Frequency in Hz
    uint16_t duration;    // Duration in milliseconds
} BuzzerEvent;

// Initializes the buzzer hardware and internal queue (must be called before use)
void Buzzer_Init(void);

// Posts a buzzer event (frequency and duration) to the buzzer task (non-blocking)
// frequency: frequency in Hz (0 means no tone)
// durationMS: duration in milliseconds
//void Buzzer_Post(uint16_t frequency, uint16_t durationMS);

// Direct control functions (optional if using Buzzer_Post and vBuzzerTask)
// Initialize PWM hardware for buzzer
void buzzer_HW_Init(void);

// Start generating buzzer tone
void buzzerStart(uint16_t freq_hz);

// Stop the buzzer tone
void buzzerStop(void);
