#pragma once

#include <stdint.h>
#include <stdbool.h>

extern "C" {
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"

#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "inc/hw_ints.h"

#include "buzzer.h"

}

//#include "button.h"
//#include "timerLib.h"
//#include "elapsedTime.h"

// Buzzer driven by PWM0 Generator 0, Output 1
#define BUZZER_PWM_BASE    PWM0_BASE
#define BUZZER_GEN         PWM_GEN_0
#define BUZZER_OUTNUM      PWM_OUT_1
#define BUZZER_OUTBIT      PWM_OUT_1_BIT

static uint32_t buzzerSysClk = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                      SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

/*
This function initializes the buzzer HW
 */
void buzzer_HW_Init()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    GPIOPinConfigure(GPIO_PF1_M0PWM1);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_64); // 120 MHz / 64 = 1.875 MHz
}


// This function generates a sound from the buzzer
void buzzerStart(uint16_t freq_hz)
{
    if (freq_hz == 0) return;

    uint32_t pwmClock = buzzerSysClk / 64;
    uint32_t period = pwmClock / freq_hz;

    PWMGenConfigure(BUZZER_PWM_BASE, BUZZER_GEN, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(BUZZER_PWM_BASE, BUZZER_GEN, period);
    PWMPulseWidthSet(BUZZER_PWM_BASE, BUZZER_OUTNUM, period / 2);
    PWMOutputState(BUZZER_PWM_BASE, BUZZER_OUTBIT, true);
    PWMGenEnable(BUZZER_PWM_BASE, BUZZER_GEN);
}

// Stop the buzzer
void buzzerStop() {
    PWMOutputState(BUZZER_PWM_BASE, BUZZER_OUTBIT, false);
}

// Init the buzzer and create the queue for buzzer event
void Buzzer_Init(void) {
    buzzer_HW_Init();
}
