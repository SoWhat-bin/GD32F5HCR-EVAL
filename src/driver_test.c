/*!
    \file    driver_test.c
    \brief   board driver self test

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

#include "driver_test.h"
#include "bus_lock.h"
#include "drivers/drivers.h"
#include "systick.h"
#include <stdio.h>

#define BUFFER_SIZE            256U
/* keep the test sector at the top of the flash, clear of the UI resource area */
#define NORFLASH_TEST_SECTOR   (NORFLASH_SIZE - NORFLASH_SECTOR_SIZE)

static uint8_t tx_buffer[BUFFER_SIZE];
static uint8_t rx_buffer[BUFFER_SIZE];

/*!
    \brief      led on/off/toggle test, all leds blink twice
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void led_test(void)
{
    uint8_t i;

    led_init(LED1);
    led_init(LED2);
    led_init(LED3);
    led_init(LED4);

    for(i = 0U; i < 2U; i++) {
        led_on(LED1);
        led_on(LED2);
        led_on(LED3);
        led_on(LED4);
        delay_1ms(200);
        led_off(LED1);
        led_off(LED2);
        led_off(LED3);
        led_off(LED4);
        delay_1ms(200);
        led_toggle(LED1);
        led_toggle(LED2);
        led_toggle(LED3);
        led_toggle(LED4);
        delay_1ms(200);
        led_off(LED1);
        led_off(LED2);
        led_off(LED3);
        led_off(LED4);
        delay_1ms(200);
    }
    printf("led test done\r\n");
}

/*!
    \brief      button state test in GPIO mode
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void button_test(void)
{
    button_init(BUTTON_TAMPER_WAKEUP, BUTTON_MODE_GPIO);
    button_init(BUTTON_USER_1, BUTTON_MODE_GPIO);
    button_init(BUTTON_USER_2, BUTTON_MODE_GPIO);

    printf("button state: TAMPER=%d USER1=%d USER2=%d (1=pressed)\r\n",
           button_pressed(BUTTON_TAMPER_WAKEUP),
           button_pressed(BUTTON_USER_1),
           button_pressed(BUTTON_USER_2));
}

/*!
    \brief      eeprom readback test, 256 bytes from 0x00
    \param[in]  none
    \param[out] none
    \retval     SUCCESS or ERROR
*/
static ErrStatus eeprom_test(void)
{
    uint16_t i;

    for(i = 0U; i < BUFFER_SIZE; i++) {
        tx_buffer[i] = (uint8_t)i;
    }

    if(!bus_lock_acquire(BUS_LOCK_TIMEOUT_MS)) {
        printf("eeprom test failed: bus busy\r\n");
        return ERROR;
    }

    eeprom_init();
    if(ERROR == eeprom_write(tx_buffer, 0U, BUFFER_SIZE)) {
        printf("eeprom test failed: write error\r\n");
        bus_lock_release();
        return ERROR;
    }
    if(ERROR == eeprom_read(rx_buffer, 0U, BUFFER_SIZE)) {
        printf("eeprom test failed: read error\r\n");
        bus_lock_release();
        return ERROR;
    }

    for(i = 0U; i < BUFFER_SIZE; i++) {
        if(rx_buffer[i] != tx_buffer[i]) {
            printf("eeprom test failed: data mismatch at 0x%02X\r\n", i);
            bus_lock_release();
            return ERROR;
        }
    }
    bus_lock_release();
    printf("eeprom test passed\r\n");
    return SUCCESS;
}

/*!
    \brief      norflash readback test, 256 bytes at the test sector
    \param[in]  none
    \param[out] none
    \retval     SUCCESS or ERROR
*/
static ErrStatus norflash_test(void)
{
    uint32_t id;
    uint16_t i;

    if(!bus_lock_acquire(BUS_LOCK_TIMEOUT_MS)) {
        printf("norflash test failed: bus busy\r\n");
        return ERROR;
    }

    id = norflash_init();
    printf("norflash id: 0x%08X\r\n", id);

    if(NORFLASH_ID != id) {
        printf("norflash test failed: unknown device\r\n");
        bus_lock_release();
        return ERROR;
    }

    norflash_sector_erase(NORFLASH_TEST_SECTOR);
    for(i = 0U; i < BUFFER_SIZE; i++) {
        tx_buffer[i] = (uint8_t)i;
    }
    norflash_write(NORFLASH_TEST_SECTOR, tx_buffer, BUFFER_SIZE);
    norflash_read(NORFLASH_TEST_SECTOR, rx_buffer, BUFFER_SIZE);
    bus_lock_release();

    for(i = 0U; i < BUFFER_SIZE; i++) {
        if(rx_buffer[i] != tx_buffer[i]) {
            printf("norflash test failed: data mismatch at 0x%04X\r\n", i);
            return ERROR;
        }
    }
    printf("norflash test passed\r\n");
    return SUCCESS;
}

/*!
    \brief      vr sample test, 5 samples of both potentiometers
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void vr_test(void)
{
    uint8_t i;

    vr_init();
    for(i = 0U; i < 5U; i++) {
        printf("vr: VR1 raw=%4d mv=%4ld  VR2 raw=%4d mv=%4ld\r\n",
               vr_raw_get(VR1), vr_millivolt_get(VR1),
               vr_raw_get(VR2), vr_millivolt_get(VR2));
        delay_1ms(500);
    }
}

/*!
    \brief      run all driver tests, results go to printf
    \param[in]  none
    \param[out] none
    \retval     none
*/
void driver_test(void)
{
    printf("\r\n===== driver test start =====\r\n");

    led_test();
    button_test();
    eeprom_test();
    norflash_test();
    vr_test();

    printf("===== driver test done =====\r\n\r\n");
}
