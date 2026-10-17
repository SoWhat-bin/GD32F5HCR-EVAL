/*!
    \file    led.c
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

#include "led.h"

static const uint32_t led_port[LEDn] = {LED1_GPIO_PORT, LED2_GPIO_PORT,
                                        LED3_GPIO_PORT, LED4_GPIO_PORT};
static const uint32_t led_pin[LEDn] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN};
static const rcu_periph_enum led_clk[LEDn] = {LED1_GPIO_CLK, LED2_GPIO_CLK,
                                              LED3_GPIO_CLK, LED4_GPIO_CLK};

/*!
    \brief      configure led GPIO
    \param[in]  led: specify the led to be configured
      \arg        LED1
      \arg        LED2
      \arg        LED3
      \arg        LED4
    \param[out] none
    \retval     none
*/
void led_init(led_t led)
{
    rcu_periph_clock_enable(led_clk[led]);
    gpio_mode_set(led_port[led], GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, led_pin[led]);
    gpio_output_options_set(led_port[led], GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, led_pin[led]);

    GPIO_BC(led_port[led]) = led_pin[led];
}

/*!
    \brief      turn on selected led
    \param[in]  led: specify the led to be turned on
    \param[out] none
    \retval     none
*/
void led_on(led_t led)
{
    GPIO_BOP(led_port[led]) = led_pin[led];
}

/*!
    \brief      turn off selected led
    \param[in]  led: specify the led to be turned off
    \param[out] none
    \retval     none
*/
void led_off(led_t led)
{
    GPIO_BC(led_port[led]) = led_pin[led];
}

/*!
    \brief      toggle the selected led
    \param[in]  led: specify the led to be toggled
    \param[out] none
    \retval     none
*/
void led_toggle(led_t led)
{
    GPIO_TG(led_port[led]) = led_pin[led];
}
