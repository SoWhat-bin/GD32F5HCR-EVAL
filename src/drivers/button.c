/*!
    \file    button.c
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

#include "button.h"

static const uint32_t button_port[BUTTONn] = {BUTTON_TAMPER_WAKEUP_GPIO_PORT,
                                              BUTTON_USER_1_GPIO_PORT,
                                              BUTTON_USER_2_GPIO_PORT};
static const uint32_t button_pin[BUTTONn] = {BUTTON_TAMPER_WAKEUP_PIN,
                                             BUTTON_USER_1_PIN,
                                             BUTTON_USER_2_PIN};
static const rcu_periph_enum button_clk[BUTTONn] = {BUTTON_TAMPER_WAKEUP_GPIO_CLK,
                                                    BUTTON_USER_1_GPIO_CLK,
                                                    BUTTON_USER_2_GPIO_CLK};
static const exti_line_enum button_exti_line[BUTTONn] = {BUTTON_TAMPER_WAKEUP_EXTI_LINE,
                                                         BUTTON_USER_1_EXTI_LINE,
                                                         BUTTON_USER_2_EXTI_LINE};
static const uint8_t button_port_source[BUTTONn] = {BUTTON_TAMPER_WAKEUP_EXTI_PORT_SOURCE,
                                                    BUTTON_USER_1_EXTI_PORT_SOURCE,
                                                    BUTTON_USER_2_EXTI_PORT_SOURCE};
static const uint8_t button_pin_source[BUTTONn] = {BUTTON_TAMPER_WAKEUP_EXTI_PIN_SOURCE,
                                                   BUTTON_USER_1_EXTI_PIN_SOURCE,
                                                   BUTTON_USER_2_EXTI_PIN_SOURCE};
static const IRQn_Type button_irqn[BUTTONn] = {BUTTON_TAMPER_WAKEUP_EXTI_IRQn,
                                               BUTTON_USER_1_EXTI_IRQn,
                                               BUTTON_USER_2_EXTI_IRQn};

/*!
    \brief      configure button GPIO
    \param[in]  button: specify the button to be configured
      \arg        BUTTON_TAMPER_WAKEUP
      \arg        BUTTON_USER_1
      \arg        BUTTON_USER_2
    \param[in]  mode: specify button mode
      \arg        BUTTON_MODE_GPIO: button is used as simple IO
      \arg        BUTTON_MODE_EXTI: button is connected to EXTI line with interrupt
    \param[out] none
    \retval     none
*/
void button_init(button_t button, button_mode_t mode)
{
    rcu_periph_clock_enable(button_clk[button]);
    rcu_periph_clock_enable(RCU_SYSCFG);

    gpio_mode_set(button_port[button], GPIO_MODE_INPUT, GPIO_PUPD_NONE, button_pin[button]);

    if(BUTTON_MODE_EXTI == mode) {
        nvic_irq_enable(button_irqn[button], 2U, 0U);
        syscfg_exti_line_config(button_port_source[button], button_pin_source[button]);
        exti_init(button_exti_line[button], EXTI_INTERRUPT, EXTI_TRIG_FALLING);
        exti_interrupt_flag_clear(button_exti_line[button]);
    }
}

/*!
    \brief      return the raw button pin level
    \param[in]  button: specify the button to be checked
    \param[out] none
    \retval     the button GPIO pin value
*/
uint8_t button_state_get(button_t button)
{
    return gpio_input_bit_get(button_port[button], button_pin[button]);
}

/*!
    \brief      return the button pressed state
    \param[in]  button: specify the button to be checked
    \param[out] none
    \retval     1 if pressed, 0 if released
*/
uint8_t button_pressed(button_t button)
{
    return (RESET == button_state_get(button)) ? 1U : 0U;
}

/*!
    \brief      weak EXTI callback, override this function to handle button events
    \param[in]  button: the button that triggered the interrupt
    \param[out] none
    \retval     none
*/
void button_exti_callback(button_t button) __attribute__((weak));
void button_exti_callback(button_t button)
{
    (void)button;
}

/*!
    \brief      this function handles EXTI line 2 interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI2_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(EXTI_2)) {
        exti_interrupt_flag_clear(EXTI_2);
        if(RESET == gpio_input_bit_get(BUTTON_TAMPER_WAKEUP_GPIO_PORT, BUTTON_TAMPER_WAKEUP_PIN)) {
            button_exti_callback(BUTTON_TAMPER_WAKEUP);
        }
        if(RESET == gpio_input_bit_get(BUTTON_USER_1_GPIO_PORT, BUTTON_USER_1_PIN)) {
            button_exti_callback(BUTTON_USER_1);
        }
    }
}

/*!
    \brief      this function handles EXTI line 3 interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI3_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(EXTI_3)) {
        exti_interrupt_flag_clear(EXTI_3);
        if(RESET == gpio_input_bit_get(BUTTON_USER_2_GPIO_PORT, BUTTON_USER_2_PIN)) {
            button_exti_callback(BUTTON_USER_2);
        }
    }
}
