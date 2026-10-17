#include "ui_platform.h"
#include "bus_lock.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "gd32w51x_f5hc.h"
#include "drivers/drivers.h"
#include <stdio.h>

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

#define UI_TEST_BUFFER_SIZE 256U
/* keep the flash test sector at the top, clear of the ui resource area */
#define UI_TEST_FLASH_SECTOR (NORFLASH_SIZE - NORFLASH_SECTOR_SIZE)
#define UI_TEST_QUEUE_DEPTH 4U
#define UI_TEST_TASK_STACK 768

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

static uint8_t test_tx[UI_TEST_BUFFER_SIZE];
static uint8_t test_rx[UI_TEST_BUFFER_SIZE];
static bool board_led1_ready;
static bool adc_ready;
static QueueHandle_t test_request_queue;
static QueueHandle_t test_result_queue;

static bool test_execute(ui_test_id_t test, char *result, uint16_t size)
{
    uint16_t i;

    switch (test) {
    case UI_TEST_EEPROM:
        for (i = 0U; i < UI_TEST_BUFFER_SIZE; i++) {
            test_tx[i] = (uint8_t)i;
        }

        if (!bus_lock_acquire(BUS_LOCK_TIMEOUT_MS)) {
            snprintf(result, size, "FAIL busy");
            return false;
        }

        eeprom_init();
        if ((ERROR == eeprom_write(test_tx, 0U, UI_TEST_BUFFER_SIZE)) ||
            (ERROR == eeprom_read(test_rx, 0U, UI_TEST_BUFFER_SIZE))) {
            bus_lock_release();
            snprintf(result, size, "FAIL bus");
            return false;
        }
        bus_lock_release();

        for (i = 0U; i < UI_TEST_BUFFER_SIZE; i++) {
            if (test_rx[i] != test_tx[i]) {
                snprintf(result, size, "FAIL 0x%02X", i);
                return false;
            }
        }

        {
            uint32_t sum = 0U;

            for (i = 0U; i < UI_TEST_BUFFER_SIZE; i++) {
                sum += test_rx[i];
            }
            snprintf(result, size, "OK 0x%04lX", (unsigned long)sum);
        }
        return true;

    case UI_TEST_NORFLASH: {
        uint32_t id;

        if (!bus_lock_acquire(BUS_LOCK_TIMEOUT_MS)) {
            snprintf(result, size, "FAIL busy");
            return false;
        }

        id = norflash_init();
        if (NORFLASH_ID != id) {
            bus_lock_release();
            snprintf(result, size, "FAIL id");
            return false;
        }

        norflash_sector_erase(UI_TEST_FLASH_SECTOR);
        for (i = 0U; i < UI_TEST_BUFFER_SIZE; i++) {
            test_tx[i] = (uint8_t)i;
        }
        norflash_write(UI_TEST_FLASH_SECTOR, test_tx, UI_TEST_BUFFER_SIZE);
        norflash_read(UI_TEST_FLASH_SECTOR, test_rx, UI_TEST_BUFFER_SIZE);
        bus_lock_release();

        for (i = 0U; i < UI_TEST_BUFFER_SIZE; i++) {
            if (test_rx[i] != test_tx[i]) {
                snprintf(result, size, "FAIL 0x%04X", i);
                return false;
            }
        }

        snprintf(result, size, "OK %08lX", (unsigned long)id);
        return true;
    }

    case UI_TEST_ADC:
        vr_init();
        snprintf(result, size, "VR1 %u/%u\nVR2 %u/%u",
                 (unsigned)vr_raw_get(VR1), (unsigned)vr_millivolt_get(VR1),
                 (unsigned)vr_raw_get(VR2), (unsigned)vr_millivolt_get(VR2));
        return true;

    default:
        snprintf(result, size, "unknown test");
        return false;
    }
}

static void test_worker_task(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        ui_test_id_t test;

        if (xQueueReceive(test_request_queue, &test, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        {
            ui_test_result_t result;

            result.test = test;
            result.pass = test_execute(test, result.text, sizeof(result.text));

            /* drop the oldest result when the ui has not collected it yet */
            if (xQueueSend(test_result_queue, &result, 0) != pdTRUE) {
                ui_test_result_t discarded;

                (void)xQueueReceive(test_result_queue, &discarded, 0);
                (void)xQueueSend(test_result_queue, &result, 0);
            }
        }
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

    test_request_queue = xQueueCreate(UI_TEST_QUEUE_DEPTH, sizeof(ui_test_id_t));
    test_result_queue = xQueueCreate(UI_TEST_QUEUE_DEPTH, sizeof(ui_test_result_t));
    if ((test_request_queue != NULL) && (test_result_queue != NULL)) {
        xTaskCreate(test_worker_task, "ui_test", UI_TEST_TASK_STACK, NULL, 1, NULL);
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

bool ui_platform_test_start(ui_test_id_t test)
{
    if ((test >= UI_TEST_COUNT) || (test_request_queue == NULL)) {
        return false;
    }

    return xQueueSend(test_request_queue, &test, 0) == pdTRUE;
}

bool ui_platform_test_result_get(ui_test_result_t *result)
{
    if ((result == NULL) || (test_result_queue == NULL)) {
        return false;
    }

    return xQueueReceive(test_result_queue, result, 0) == pdTRUE;
}

bool ui_platform_adc_sample(uint16_t *vr1_raw, uint32_t *vr1_millivolt,
                             uint16_t *vr2_raw, uint32_t *vr2_millivolt)
{
    if (!adc_ready) {
        vr_init();
        adc_ready = true;
    }

    *vr1_raw = vr_raw_get(VR1);
    *vr1_millivolt = vr_millivolt_get(VR1);
    *vr2_raw = vr_raw_get(VR2);
    *vr2_millivolt = vr_millivolt_get(VR2);
    return true;
}

void ui_platform_led_set(bool on)
{
    if (!board_led1_ready) {
        led_init(LED1);
        board_led1_ready = true;
    }

    if (on) {
        led_on(LED1);
    } else {
        led_off(LED1);
    }
}
