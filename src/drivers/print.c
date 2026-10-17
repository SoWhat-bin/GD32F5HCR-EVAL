/*!
    \file    print.c
    \brief   usart print for GD32F5HCR-EVAL

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

#include "print.h"
#include <stdio.h>

/*!
    \brief      initialize EVAL_COM0, USART2 @ 115200 8N1
    \param[in]  none
    \param[out] none
    \retval     none
*/
void print_init(void)
{
    rcu_periph_clock_enable(PRINT_GPIO_CLK);
    rcu_periph_clock_enable(PRINT_USART_CLK);

    gpio_af_set(PRINT_GPIO_PORT, PRINT_GPIO_AF, PRINT_TX_PIN | PRINT_RX_PIN);
    gpio_mode_set(PRINT_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
                  PRINT_TX_PIN | PRINT_RX_PIN);
    gpio_output_options_set(PRINT_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ,
                            PRINT_TX_PIN | PRINT_RX_PIN);

    usart_deinit(PRINT_USART);
    usart_baudrate_set(PRINT_USART, 115200U);
    usart_receive_config(PRINT_USART, USART_RECEIVE_ENABLE);
    usart_transmit_config(PRINT_USART, USART_TRANSMIT_ENABLE);
    usart_enable(PRINT_USART);
}

/*!
    \brief      retarget the C library printf to EVAL_COM0
    \param[in]  ch: character to be transmitted
    \param[out] none
    \retval     transmitted character
*/
int __io_putchar(int ch)
{
    usart_data_transmit(PRINT_USART, (uint8_t)ch);
    while(RESET == usart_flag_get(PRINT_USART, USART_FLAG_TBE)) {
    }
    return ch;
}
