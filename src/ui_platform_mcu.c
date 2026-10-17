#include "ui_platform.h"

#include "FreeRTOS.h"
#include "task.h"
#include "gd32w51x_f5hc.h"

#define RTC_BACKUP_MAGIC 0x32F0U
#define RTC_ASYNCH_PRESCALER 0x63U
#define RTC_SYNCH_PRESCALER 0x13FU

#define UI_KEY_FOCUS_NEXT_PORT GPIOD
#define UI_KEY_FOCUS_NEXT_PIN GPIO_PIN_2
#define UI_KEY_FOCUS_NEXT_CLK RCU_GPIOD

#define UI_KEY_ACTIVATE_PORT GPIOA
#define UI_KEY_ACTIVATE_PIN GPIO_PIN_3
#define UI_KEY_ACTIVATE_CLK RCU_GPIOA

#define UI_KEY_BACK_PORT GPIOA
#define UI_KEY_BACK_PIN GPIO_PIN_2
#define UI_KEY_BACK_CLK RCU_GPIOA

static bool rtc_ready;

static uint8_t decimal_to_bcd(uint8_t value)
{
    return (uint8_t)(((value / 10U) << 4U) | (value % 10U));
}

static uint8_t bcd_to_decimal(uint8_t value)
{
    return (uint8_t)(((value >> 4U) * 10U) + (value & 0x0FU));
}

static uint8_t build_month(void)
{
    if (__DATE__[0] == 'J' && __DATE__[1] == 'a') return 1U;
    if (__DATE__[0] == 'F') return 2U;
    if (__DATE__[0] == 'M' && __DATE__[2] == 'r') return 3U;
    if (__DATE__[0] == 'A' && __DATE__[1] == 'p') return 4U;
    if (__DATE__[0] == 'M') return 5U;
    if (__DATE__[0] == 'J' && __DATE__[1] == 'u') return 6U;
    if (__DATE__[0] == 'J') return 7U;
    if (__DATE__[0] == 'A') return 8U;
    if (__DATE__[0] == 'S') return 9U;
    if (__DATE__[0] == 'O') return 10U;
    if (__DATE__[0] == 'N') return 11U;
    return 12U;
}

static void key_pin_init(uint32_t port, uint32_t pin, uint32_t clock)
{
    rcu_periph_clock_enable(clock);
    gpio_mode_set(port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, pin);
}

static void rtc_init_from_build_time(void)
{
    rtc_parameter_struct rtc_time = {0};

    rtc_time.factor_asyn = RTC_ASYNCH_PRESCALER;
    rtc_time.factor_syn = RTC_SYNCH_PRESCALER;
    rtc_time.year = decimal_to_bcd((uint8_t)(((__DATE__[9] - '0') * 10U) + (__DATE__[10] - '0')));
    rtc_time.month = decimal_to_bcd(build_month());
    rtc_time.date = decimal_to_bcd((uint8_t)((__DATE__[4] == ' ' ? 0U : (__DATE__[4] - '0') * 10U) + (__DATE__[5] - '0')));
    rtc_time.day_of_week = RTC_MONDAY;
    rtc_time.hour = decimal_to_bcd((uint8_t)((__TIME__[0] - '0') * 10U + (__TIME__[1] - '0')));
    rtc_time.minute = decimal_to_bcd((uint8_t)((__TIME__[3] - '0') * 10U + (__TIME__[4] - '0')));
    rtc_time.second = decimal_to_bcd((uint8_t)((__TIME__[6] - '0') * 10U + (__TIME__[7] - '0')));
    rtc_time.display_format = RTC_24HOUR;
    rtc_time.am_pm = RTC_AM;

    rtc_ready = (SUCCESS == rtc_init(&rtc_time));
    if (rtc_ready) {
        RTC_BKP0 = RTC_BACKUP_MAGIC;
    }
}

void ui_platform_init(void)
{
    key_pin_init(UI_KEY_FOCUS_NEXT_PORT, UI_KEY_FOCUS_NEXT_PIN, UI_KEY_FOCUS_NEXT_CLK);
    key_pin_init(UI_KEY_ACTIVATE_PORT, UI_KEY_ACTIVATE_PIN, UI_KEY_ACTIVATE_CLK);
    key_pin_init(UI_KEY_BACK_PORT, UI_KEY_BACK_PIN, UI_KEY_BACK_CLK);

    bool oscillator_ready;
    bool registers_ready;

    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();
    rcu_osci_on(RCU_IRC32K);
    oscillator_ready = (SUCCESS == rcu_osci_stab_wait(RCU_IRC32K));
    rcu_rtc_clock_config(RCU_RTCSRC_IRC32K);
    rcu_periph_clock_enable(RCU_RTC);
    registers_ready = (SUCCESS == rtc_register_sync_wait());

    if (!oscillator_ready || !registers_ready) {
        rtc_ready = false;
        return;
    }

    if ((RTC_BKP0 != RTC_BACKUP_MAGIC) ||
        (RCU_RTCSRC_NONE == (RCU_BDCTL & RCU_BDCTL_RTCSRC))) {
        rtc_init_from_build_time();
    } else {
        ui_platform_time_t current_time;

        rtc_ready = true;
        rtc_ready = ui_platform_time_get(&current_time);
    }
}

uint32_t ui_platform_millis(void)
{
    return (uint32_t)xTaskGetTickCount();
}

bool ui_platform_time_get(ui_platform_time_t *time)
{
    rtc_parameter_struct rtc_time;

    if ((time == NULL) || !rtc_ready) {
        return false;
    }

    rtc_current_time_get(&rtc_time);
    time->year = (uint16_t)(2000U + bcd_to_decimal(rtc_time.year));
    time->month = bcd_to_decimal(rtc_time.month);
    time->day = bcd_to_decimal(rtc_time.date);
    time->hour = bcd_to_decimal(rtc_time.hour);
    time->minute = bcd_to_decimal(rtc_time.minute);
    time->second = bcd_to_decimal(rtc_time.second);

    if (!ui_platform_time_valid(time)) {
        rtc_ready = false;
        return false;
    }

    return true;
}

bool ui_platform_key_pressed(ui_key_id_t key)
{
    switch (key) {
    case UI_KEY_FOCUS_NEXT:
        return gpio_input_bit_get(UI_KEY_FOCUS_NEXT_PORT, UI_KEY_FOCUS_NEXT_PIN) == RESET;
    case UI_KEY_ACTIVATE:
        return gpio_input_bit_get(UI_KEY_ACTIVATE_PORT, UI_KEY_ACTIVATE_PIN) == RESET;
    case UI_KEY_BACK:
        return gpio_input_bit_get(UI_KEY_BACK_PORT, UI_KEY_BACK_PIN) == RESET;
    default:
        return false;
    }
}
