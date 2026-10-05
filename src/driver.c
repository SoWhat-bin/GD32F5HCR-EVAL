#include "driver.h"
uint8_t spi_write_byte(uint32_t spi_periph, uint8_t byte)
{
    __IO uint8_t data;
    while (RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TBE))
        ;
    spi_i2s_data_transmit(spi_periph, (uint16_t)byte);
    while (RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE))
        ;
    data = (uint8_t)spi_i2s_data_receive(spi_periph);
    return (data);
}

uint16_t rx_array[1];
void spi_lcd_init(void)
{
    spi_parameter_struct spi_init_struct;
    dma_multi_data_parameter_struct dma_init_parameter;
    /* enable the GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    /* enable the SPI1 clock */
    rcu_periph_clock_enable(RCU_SPI1);
    /* peripheral clock enable */
    rcu_periph_clock_enable(RCU_DMA0);

    /* SPI1 GPIO configuration: SCK/PC11, MISO/PC13, MOSI/PD0 */
    gpio_af_set(GPIOC, GPIO_AF_9, GPIO_PIN_13 | GPIO_PIN_11);
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13 | GPIO_PIN_11);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ, GPIO_PIN_13 | GPIO_PIN_11);

    /* MOSI/PD0 */
    gpio_af_set(GPIOD, GPIO_AF_9, GPIO_PIN_0);
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_0);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ, GPIO_PIN_0);

    /* GPIOB configuration (LCD_RST/PB1, LCD_DC/PB2) */
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_1 | GPIO_PIN_2);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ, GPIO_PIN_1 | GPIO_PIN_2);

    /* GPIOC configuration (LCD_RST/PC3) */
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_3);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_166MHZ, GPIO_PIN_3);

    LCD_CS_SET;
    LCD_RST_SET;
    delay_1ms(5);
    LCD_RST_CLR;
    delay_1ms(20);
    LCD_RST_SET;
    delay_1ms(120);

    /* SPI1 parameter configuration */
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.prescale = SPI_PSC_2;
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init(SPI1, &spi_init_struct);

    /* DMA peripheral configure */
    dma_deinit(DMA0, DMA_CH4);
    dma_multi_data_para_struct_init(&dma_init_parameter);
    dma_init_parameter.memory0_addr = 0U;
    dma_init_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_parameter.memory_burst_width = DMA_MEMORY_BURST_4_BEAT;
    dma_init_parameter.periph_addr = (uint32_t)&SPI_DATA(SPI1);
    dma_init_parameter.periph_width = DMA_PERIPH_WIDTH_16BIT;
    dma_init_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_parameter.periph_burst_width = DMA_PERIPH_BURST_SINGLE;
    dma_init_parameter.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_parameter.critical_value = DMA_FIFO_2_WORD;
    dma_init_parameter.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    dma_init_parameter.number = 0;
    dma_init_parameter.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_multi_data_mode_init(DMA0, DMA_CH4, &dma_init_parameter);
    /* connect DMA0_CH4 to SPI1 TX */
    dma_channel_subperipheral_select(DMA0, DMA_CH4, DMA_SUBPERI0);
    /* interrupt configure */
    dma_interrupt_enable(DMA0, DMA_CH4, DMA_INT_FTF);
    nvic_irq_enable(DMA0_Channel4_IRQn, 0, 0);

    /* DMA peripheral configure */
    dma_deinit(DMA0, DMA_CH3);
    dma_multi_data_para_struct_init(&dma_init_parameter);
    dma_init_parameter.memory0_addr = (uint32_t)rx_array;
    dma_init_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init_parameter.memory_inc = DMA_MEMORY_INCREASE_DISABLE;
    dma_init_parameter.memory_burst_width = DMA_MEMORY_BURST_SINGLE;
    dma_init_parameter.periph_addr = (uint32_t)&SPI_DATA(SPI1);
    dma_init_parameter.periph_width = DMA_PERIPH_WIDTH_16BIT;
    dma_init_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_parameter.periph_burst_width = DMA_PERIPH_BURST_SINGLE;
    dma_init_parameter.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_parameter.critical_value = DMA_FIFO_1_WORD;
    dma_init_parameter.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    dma_init_parameter.number = 0U;
    dma_init_parameter.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_multi_data_mode_init(DMA0, DMA_CH3, &dma_init_parameter);
    /* connect DMA0_CH3 to SPI1 RX */
    dma_channel_subperipheral_select(DMA0, DMA_CH3, DMA_SUBPERI0);
    /* interrupt configure */
    dma_interrupt_enable(DMA0, DMA_CH3, DMA_INT_FTF);
    nvic_irq_enable(DMA0_Channel3_IRQn, 0, 0);
    spi_dma_enable(SPI1, SPI_DMA_TRANSMIT);
    spi_dma_enable(SPI1, SPI_DMA_RECEIVE);
    spi_enable(SPI1);

    LCD_CS_CLR;

    /* write the register address 0xCB*/
    lcd_write_cmd(0xCB);
    lcd_write_data_8bit(0x39);
    lcd_write_data_8bit(0x2C);
    lcd_write_data_8bit(0x00);
    lcd_write_data_8bit(0x34);
    lcd_write_data_8bit(0x02);

    /* write the register address 0xCF*/
    lcd_write_cmd(0xCF);
    lcd_write_data_8bit(0x00);
    lcd_write_data_8bit(0XC1);
    lcd_write_data_8bit(0X30);

    /* write the register address 0xE8*/
    lcd_write_cmd(0xE8);
    lcd_write_data_8bit(0x85);
    lcd_write_data_8bit(0x00);
    lcd_write_data_8bit(0x78);

    /* write the register address 0xEA*/
    lcd_write_cmd(0xEA);
    lcd_write_data_8bit(0x00);
    lcd_write_data_8bit(0x00);

    /* write the register address 0xED*/
    lcd_write_cmd(0xED);
    lcd_write_data_8bit(0x64);
    lcd_write_data_8bit(0x03);
    lcd_write_data_8bit(0X12);
    lcd_write_data_8bit(0X81);

    /* write the register address 0xF7*/
    lcd_write_cmd(0xF7);
    lcd_write_data_8bit(0x20);

    /* power control VRH[5:0] */
    lcd_write_cmd(0xC0);
    lcd_write_data_8bit(0x23);

    /* power control SAP[2:0];BT[3:0] */
    lcd_write_cmd(0xC1);
    lcd_write_data_8bit(0x10);

    /* vcm control */
    lcd_write_cmd(0xC5);
    lcd_write_data_8bit(0x3e);
    lcd_write_data_8bit(0x28);

    /* vcm control2 */
    lcd_write_cmd(0xC7);
    lcd_write_data_8bit(0x86);

    lcd_write_cmd(0x36);
    lcd_write_data_8bit(0xE8);

    /* write the register address 0x3A*/
    lcd_write_cmd(0x3A);
    lcd_write_data_8bit(0x55);

    /* write the register address 0xB1*/
    lcd_write_cmd(0xB1);
    lcd_write_data_8bit(0x00);
    lcd_write_data_8bit(0x10);

    /* display function control */
    lcd_write_cmd(0xB6);
    lcd_write_data_8bit(0x08);
    lcd_write_data_8bit(0x82);
    lcd_write_data_8bit(0x27);

    /* 3gamma function disable */
    lcd_write_cmd(0xF2);
    lcd_write_data_8bit(0x00);

    /* gamma curve selected  */
    lcd_write_cmd(0x26);
    lcd_write_data_8bit(0x01);

    /* set gamma  */
    lcd_write_cmd(0xE0);
    lcd_write_data_8bit(0x0F);
    lcd_write_data_8bit(0x31);
    lcd_write_data_8bit(0x2B);
    lcd_write_data_8bit(0x0C);
    lcd_write_data_8bit(0x0E);
    lcd_write_data_8bit(0x08);
    lcd_write_data_8bit(0x4E);
    lcd_write_data_8bit(0xF1);
    lcd_write_data_8bit(0x37);
    lcd_write_data_8bit(0x07);
    lcd_write_data_8bit(0x10);
    lcd_write_data_8bit(0x03);
    lcd_write_data_8bit(0x0E);
    lcd_write_data_8bit(0x09);
    lcd_write_data_8bit(0x00);

    /* set gamma  */
    lcd_write_cmd(0XE1);
    lcd_write_data_8bit(0x00);
    lcd_write_data_8bit(0x0E);
    lcd_write_data_8bit(0x14);
    lcd_write_data_8bit(0x03);
    lcd_write_data_8bit(0x11);
    lcd_write_data_8bit(0x07);
    lcd_write_data_8bit(0x31);
    lcd_write_data_8bit(0xC1);
    lcd_write_data_8bit(0x48);
    lcd_write_data_8bit(0x08);
    lcd_write_data_8bit(0x0F);
    lcd_write_data_8bit(0x0C);
    lcd_write_data_8bit(0x31);
    lcd_write_data_8bit(0x36);
    lcd_write_data_8bit(0x0F);

    /* exit sleep */
    lcd_write_cmd(0x11);
    delay_1ms(120);

    /* display on */
    lcd_write_cmd(0x29);
    lcd_write_cmd(0x2c);
}
void lcd_write_cmd(uint8_t cmd)
{
    LCD_RS_CLR;
    spi_write_byte(SPI1, cmd);
}
void lcd_write_data_8bit(uint8_t data)
{
    LCD_RS_SET;
    spi_write_byte(SPI1, data);
}
void lcd_write_data_16bit(uint8_t datah, uint8_t datal)
{
    lcd_write_data_8bit(datah);
    lcd_write_data_8bit(datal);
}
void lcd_set_region(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    LCD_CS_CLR;

    /* write the register address 0x2A*/
    lcd_write_cmd(0x2A);
    lcd_write_data_16bit(x_start >> 8, x_start);
    lcd_write_data_16bit(x_end >> 8, x_end);

    /* write the register address 0x2B*/
    lcd_write_cmd(0x2B);
    lcd_write_data_16bit(y_start >> 8, y_start);
    lcd_write_data_16bit(y_end >> 8, y_end);

    /* write the register address 0x2C*/
    lcd_write_cmd(0x2C);
    LCD_CS_SET;
}