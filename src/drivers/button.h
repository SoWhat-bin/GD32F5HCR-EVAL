/*!
    \file    button.h
    \brief   button driver for GD32F5HCR-EVAL

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

#ifndef BUTTON_H
#define BUTTON_H

#include "gd32w51x_f5hc.h"

#define BUTTONn 3U

/* tamper push-button, PA2, shares EXTI line 2 with USER1 */
#define BUTTON_TAMPER_WAKEUP_PIN                 GPIO_PIN_2
#define BUTTON_TAMPER_WAKEUP_GPIO_PORT           GPIOA
#define BUTTON_TAMPER_WAKEUP_GPIO_CLK            RCU_GPIOA
#define BUTTON_TAMPER_WAKEUP_EXTI_LINE           EXTI_2
#define BUTTON_TAMPER_WAKEUP_EXTI_PORT_SOURCE    EXTI_SOURCE_GPIOA
#define BUTTON_TAMPER_WAKEUP_EXTI_PIN_SOURCE     EXTI_SOURCE_PIN2
#define BUTTON_TAMPER_WAKEUP_EXTI_IRQn           EXTI2_IRQn

/* user key1 push-button, PD2, shares EXTI line 2 with TAMPER */
#define BUTTON_USER_1_PIN                        GPIO_PIN_2
#define BUTTON_USER_1_GPIO_PORT                  GPIOD
#define BUTTON_USER_1_GPIO_CLK                   RCU_GPIOD
#define BUTTON_USER_1_EXTI_LINE                  EXTI_2
#define BUTTON_USER_1_EXTI_PORT_SOURCE           EXTI_SOURCE_GPIOD
#define BUTTON_USER_1_EXTI_PIN_SOURCE            EXTI_SOURCE_PIN2
#define BUTTON_USER_1_EXTI_IRQn                  EXTI2_IRQn

/* user key2 push-button, PA3 */
#define BUTTON_USER_2_PIN                        GPIO_PIN_3
#define BUTTON_USER_2_GPIO_PORT                  GPIOA
#define BUTTON_USER_2_GPIO_CLK                   RCU_GPIOA
#define BUTTON_USER_2_EXTI_LINE                  EXTI_3
#define BUTTON_USER_2_EXTI_PORT_SOURCE           EXTI_SOURCE_GPIOA
#define BUTTON_USER_2_EXTI_PIN_SOURCE            EXTI_SOURCE_PIN3
#define BUTTON_USER_2_EXTI_IRQn                  EXTI3_IRQn

typedef enum {
    BUTTON_TAMPER_WAKEUP = 0U,
    BUTTON_USER_1 = 1U,
    BUTTON_USER_2 = 2U
} button_t;

typedef enum {
    BUTTON_MODE_GPIO = 0U,
    BUTTON_MODE_EXTI = 1U
} button_mode_t;

/* function declarations */
/* configure button GPIO, falling edge interrupt when EXTI mode.
   TAMPER and USER1 share EXTI line 2, do not use EXTI mode for both */
void button_init(button_t button, button_mode_t mode);
/* return the raw button pin level */
uint8_t button_state_get(button_t button);
/* return 1 if the button is pressed, buttons are active low */
uint8_t button_pressed(button_t button);
/* weak callback invoked from EXTI handlers, override to handle button events */
void button_exti_callback(button_t button);

#endif /* BUTTON_H */
