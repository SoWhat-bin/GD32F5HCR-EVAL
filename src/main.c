/*!
    \file    main.c
    \brief   led spark with systick, USART print and key example

    \version 2026-04-15, V2.0.0, firmware for gd32w51x_f5hc
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

#include "main.h"
#include "ui_init.h"
#include "drivers/print.h"
#include "driver_test.h"
#include "bus_lock.h"

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

static void vApplicationTask(void *pvParameters)
{
    (void)pvParameters;

    printf("\r\nCK_SYS is %d", rcu_clock_freq_get(CK_SYS));
    printf("\r\nCK_AHB is %d", rcu_clock_freq_get(CK_AHB));
    printf("\r\nCK_APB1 is %d", rcu_clock_freq_get(CK_APB1));
    printf("\r\nCK_APB2 is %d", rcu_clock_freq_get(CK_APB2));

    for (;;)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

static void driverTestTask(void *pvParameters)
{
    (void)pvParameters;

    driver_test();
    vTaskDelete(NULL);
}

int main(void)
{
#ifdef __FIRMWARE_VERSION_DEFINE
    uint32_t fw_ver = 0;
#endif

    lv_init();
    systick_config();
    print_init();
    /* the driver and ui self tests share i2c1 and sqpi */
    bus_lock_init();
    lv_port_disp_init();

#ifdef __FIRMWARE_VERSION_DEFINE
    fw_ver = gd32w51x_f5hc_firmware_version_get();
    printf("\r\ngd32w51x_f5hc series firmware version: V%d.%d.%d", (uint8_t)(fw_ver >> 24), (uint8_t)(fw_ver >> 16),
           (uint8_t)(fw_ver >> 8));
#endif /* __FIRMWARE_VERSION_DEFINE */

    ui_init();

    if (xTaskCreate(vApplicationTask, "lvgl", 2048, NULL, 1, NULL) == pdPASS)
    {
        /* self tests run in the background so boot and tests never stall the ui */
        xTaskCreate(driverTestTask, "dtest", 1024, NULL, 1, NULL);
        vTaskStartScheduler();
    }
    else
    {
        printf("\r\nUI task allocation failed");
    }

    while (1)
    {
        __NOP();
    }
}
