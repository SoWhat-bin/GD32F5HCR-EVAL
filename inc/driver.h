#ifndef __DRIVER_H__
#define __DRIVER_H__
#include "gd32w51x_f5hc_gpio.h"
#include "systick.h"
/* PB1 tft rst */
#define LCD_RST_SET ((uint32_t)(GPIO_BOP(GPIOB) = GPIO_PIN_1))
#define LCD_RST_CLR ((uint32_t)(GPIO_BC(GPIOB) = GPIO_PIN_1))

/* PB2 tft rs/dc */
#define LCD_RS_SET ((uint32_t)(GPIO_BOP(GPIOB) = GPIO_PIN_2))
#define LCD_RS_CLR ((uint32_t)(GPIO_BC(GPIOB) = GPIO_PIN_2))

/* PC3 tft cs */
#define LCD_CS_SET ((uint32_t)(GPIO_BOP(GPIOC) = GPIO_PIN_3))
#define LCD_CS_CLR ((uint32_t)(GPIO_BC(GPIOC) = GPIO_PIN_3))
void lcd_write_cmd(uint8_t cmd);
void lcd_write_data_8bit(uint8_t data);
void spi_lcd_init(void);
void lcd_write_data_16bit(uint8_t datah, uint8_t datal);
void lcd_set_region(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);
#endif
