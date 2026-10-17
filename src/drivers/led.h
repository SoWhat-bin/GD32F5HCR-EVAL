/*!
    \file    led.h
    \brief   led driver for GD32F5HCR-EVAL

    \version 2026-10-17, V1.0.0
*/

/*
    Copyright (c) 2026, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#ifndef LED_H
#define LED_H

#include "gd32w51x_f5hc.h"

#define LEDn      4U

#define LED1_PIN  GPIO_PIN_6
#define LED1_GPIO_PORT  GPIOB
#define LED1_GPIO_CLK   RCU_GPIOB

#define LED2_PIN  GPIO_PIN_15
#define LED2_GPIO_PORT  GPIOA
#define LED2_GPIO_CLK   RCU_GPIOA

#define LED3_PIN  GPIO_PIN_6
#define LED3_GPIO_PORT  GPIOA
#define LED3_GPIO_CLK   RCU_GPIOA

#define LED4_PIN  GPIO_PIN_1
#define LED4_GPIO_PORT  GPIOA
#define LED4_GPIO_CLK   RCU_GPIOA

typedef enum {
    LED1 = 0U,
    LED2 = 1U,
    LED3 = 2U,
    LED4 = 3U
} led_t;

/* function declarations */
/* configure led GPIO, led is off after init */
void led_init(led_t led);
/* turn on selected led */
void led_on(led_t led);
/* turn off selected led */
void led_off(led_t led);
/* toggle the selected led */
void led_toggle(led_t led);

#endif /* LED_H */
