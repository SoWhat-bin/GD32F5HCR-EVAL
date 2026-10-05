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

static lv_obj_t *test_bar;
static lv_obj_t *refresh_label;

static void lv_test_ui_init(void)
{
    static const char *swatch_names[] = {"RED", "GREEN", "BLUE"};
    static const uint32_t swatch_colors[] = {0xD94B4B, 0x36A879, 0x3979D6};
    lv_obj_t *screen = lv_scr_act();

    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101A22), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_t *header = lv_obj_create(screen);
    lv_obj_remove_style_all(header);
    lv_obj_set_size(header, 296, 48);
    lv_obj_set_pos(header, 12, 10);
    lv_obj_set_style_radius(header, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x20343D), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "GD32 LVGL DISPLAY TEST");
    lv_obj_set_style_text_color(title, lv_color_hex(0xF1F5F4), LV_PART_MAIN);
    lv_obj_set_pos(title, 10, 4);

    lv_obj_t *subtitle = lv_label_create(header);
    lv_label_set_text(subtitle, "320 x 240  |  RGB565  |  SPI1 + DMA");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xA9BEC0), LV_PART_MAIN);
    lv_obj_set_pos(subtitle, 10, 25);

    for (uint32_t i = 0; i < 3; i++)
    {
        lv_obj_t *swatch = lv_obj_create(screen);
        lv_obj_remove_style_all(swatch);
        lv_obj_set_size(swatch, 94, 38);
        lv_obj_set_pos(swatch, 12 + (int32_t)i * 101, 70);
        lv_obj_set_style_radius(swatch, 4, LV_PART_MAIN);
        lv_obj_set_style_bg_color(swatch, lv_color_hex(swatch_colors[i]), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(swatch, LV_OPA_COVER, LV_PART_MAIN);

        lv_obj_t *swatch_label = lv_label_create(swatch);
        lv_label_set_text(swatch_label, swatch_names[i]);
        lv_obj_set_style_text_color(swatch_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_center(swatch_label);
    }

    lv_obj_t *bar_title = lv_label_create(screen);
    lv_label_set_text(bar_title, "LIVE PARTIAL REFRESH");
    lv_obj_set_style_text_color(bar_title, lv_color_hex(0xC6D2D2), LV_PART_MAIN);
    lv_obj_set_pos(bar_title, 12, 122);

    test_bar = lv_bar_create(screen);
    lv_obj_set_size(test_bar, 296, 16);
    lv_obj_set_pos(test_bar, 12, 146);
    lv_bar_set_range(test_bar, 0, 100);
    lv_bar_set_value(test_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(test_bar, lv_color_hex(0x293A40), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(test_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(test_bar, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_color(test_bar, lv_color_hex(0x43D6A2), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(test_bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(test_bar, 4, LV_PART_INDICATOR);

    refresh_label = lv_label_create(screen);
    lv_label_set_text(refresh_label, "DMA refresh 000000");
    lv_obj_set_style_text_color(refresh_label, lv_color_hex(0xF1F5F4), LV_PART_MAIN);
    lv_obj_set_pos(refresh_label, 12, 180);

    lv_obj_t *footer = lv_label_create(screen);
    lv_label_set_text(footer, "Visual test only  |  Touch input not configured");
    lv_obj_set_style_text_color(footer, lv_color_hex(0x83999D), LV_PART_MAIN);
    lv_obj_set_pos(footer, 12, 211);
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

static void vApplicationTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t refresh_count = 0;
    uint32_t frame = 0;
    int32_t progress = 0;
    int32_t direction = 5;

    printf("\r\nCK_SYS is %d", rcu_clock_freq_get(CK_SYS));
    printf("\r\nCK_AHB is %d", rcu_clock_freq_get(CK_AHB));
    printf("\r\nCK_APB1 is %d", rcu_clock_freq_get(CK_APB1));
    printf("\r\nCK_APB2 is %d", rcu_clock_freq_get(CK_APB2));

    lv_test_ui_init();

    for (;;)
    {
        lv_timer_handler();
        if ((frame++ % 20U) == 0U)
        {
            progress += direction;
            if (progress >= 100 || progress <= 0)
            {
                direction = -direction;
            }
            lv_bar_set_value(test_bar, progress, LV_ANIM_OFF);
            lv_label_set_text_fmt(refresh_label, "DMA refresh %06lu", (unsigned long)refresh_count++);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

int main(void)
{

#ifdef __FIRMWARE_VERSION_DEFINE
    uint32_t fw_ver = 0;
#endif
    lv_init();
    systick_config();
    lv_port_disp_init();
#ifdef __FIRMWARE_VERSION_DEFINE
    fw_ver = gd32w51x_f5hc_firmware_version_get();
    /* print firmware version */
    printf("\r\ngd32w51x_f5hc series firmware version: V%d.%d.%d", (uint8_t)(fw_ver >> 24), (uint8_t)(fw_ver >> 16),
           (uint8_t)(fw_ver >> 8));
#endif /* __FIRMWARE_VERSION_DEFINE */

    if (xTaskCreate(vApplicationTask, "lvgl", 1024, NULL, 1, NULL) == pdPASS)
    {
        vTaskStartScheduler();
    }

    while (1)
    {
    }
}
