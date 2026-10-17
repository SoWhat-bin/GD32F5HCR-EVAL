#include "lvgl.h"

#include <stdint.h>

uint32_t lv_tick_get(void)
{
    return 1234U;
}

int main(void)
{
    LV_LOG_WARN("integration log path");
    return 0;
}
