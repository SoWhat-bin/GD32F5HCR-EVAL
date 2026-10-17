/*!
    \file    norflash.c
    \brief   norflash driver for GD25Q16 on GD32F5HCR-EVAL

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

#include "norflash.h"
#include "gd32w51x_f5hc_sqpi.h"
#include "systick.h"
#include <stdio.h>

#define NORFLASH_WREN          0x06U
#define NORFLASH_WRDI          0x04U
#define NORFLASH_WRSR          0x01U
#define NORFLASH_RDSR          0x05U
#define NORFLASH_READ          0x03U
#define NORFLASH_FASTREAD      0x0BU
#define NORFLASH_QUADREAD      0x6BU
#define NORFLASH_WRITE         0x02U
#define NORFLASH_QUADWRITE     0x32U
#define NORFLASH_SE            0x20U
#define NORFLASH_CE            0xC7U
#define NORFLASH_RDID          0x9FU
#define NORFLASH_WIP_FLAG      0x01U
#define NORFLASH_WEL_FLAG      0x02U

#define SQPI_ADDR_0BIT         (SQPI_INIT & (~(uint32_t)0x1F000000U))
#define SQPI_ADDR_16BITS       (SQPI_ADDR_0BIT | 0x10000000U)
#define SQPI_ADDR_24BITS       (SQPI_ADDR_0BIT | 0x18000000U)

static void norflash_state_check(uint8_t mask, uint8_t check);
static void norflash_page_write(uint32_t write_addr, uint8_t *write_buff, uint32_t size);
static void norflash_write_enable(void);

/*!
    \brief      check SQPI status register
    \param[in]  mask: mask value
    \param[in]  check: check value
    \param[out] none
    \retval     none
*/
static void norflash_state_check(uint8_t mask, uint8_t check)
{
    volatile uint32_t temp = 0U;
    SQPI_INIT = SQPI_ADDR_0BIT;
    sqpi_read_command_config(SQPI_MODE_SSS, 0U, NORFLASH_RDSR);

    do {
        temp = *(uint8_t *)SQPI_LOGIC_ADDR;
    } while((temp & mask) != check);
}

/*!
    \brief      write more than one byte to the flash
    \param[in]  write_addr: flash's internal address to write to
    \param[in]  write_buff: pointer to the buffer
    \param[in]  size: number of bytes to write to the flash
    \param[out] none
    \retval     none
*/
static void norflash_page_write(uint32_t write_addr, uint8_t *write_buff, uint32_t size)
{
    uint32_t i = 0U;

    for(i = 0U; i < size; i++) {
        norflash_write_enable();
        SQPI_INIT = SQPI_ADDR_24BITS;
        sqpi_write_command_config(SQPI_MODE_SSQ, 0U, NORFLASH_QUADWRITE);
        SQPI_INIT = SQPI_ADDR_24BITS;
        *(uint8_t *)(SQPI_LOGIC_ADDR + write_addr + i) = *write_buff;
        write_buff++;
        norflash_state_check(NORFLASH_WIP_FLAG, 0x00U);
    }
}

/*!
    \brief      enable the write access to the flash
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void norflash_write_enable(void)
{
    SQPI_INIT = SQPI_ADDR_0BIT;
    sqpi_write_command_config(SQPI_MODE_SSS, 0U, NORFLASH_WREN);
    sqpi_special_command();
    norflash_state_check(NORFLASH_WEL_FLAG, 0x02U);
}

/*!
    \brief      configure SQPI GPIO in SPI mode before quad enable
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void norflash_gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);

    gpio_af_set(GPIOA, GPIO_AF_3, GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11);
    gpio_af_set(GPIOA, GPIO_AF_8, GPIO_PIN_12);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_3 | GPIO_PIN_4);

    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_3 | GPIO_PIN_4);

    gpio_bit_set(GPIOB, GPIO_PIN_4);
    gpio_bit_reset(GPIOB, GPIO_PIN_3);
}

/*!
    \brief      switch PB3/PB4 to SQPI IO2/IO3 after quad enable
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void norflash_gpio_quad_switch(void)
{
    gpio_af_set(GPIOB, GPIO_AF_8, GPIO_PIN_3 | GPIO_PIN_4);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3 | GPIO_PIN_4);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_3 | GPIO_PIN_4);
}

/*!
    \brief      initialize SQPI and the GD25Q16 flash, quad mode is enabled when the ID matches
    \param[in]  none
    \param[out] none
    \retval     flash identification, NORFLASH_ID on success
*/
uint32_t norflash_init(void)
{
    sqpi_parameter_struct sqpi_para;
    uint32_t id;

    norflash_gpio_config();

    rcu_periph_clock_enable(RCU_SQPI);
    sqpi_para.addr_bit = 0U;
    sqpi_para.clk_div = 8U;
    sqpi_para.cmd_bit = SQPI_CMDBIT_8_BITS;
    sqpi_para.id_length = SQPI_ID_LENGTH_32_BITS;
    sqpi_para.polarity = SQPI_SAMPLE_POLARITY_RISING;
    sqpi_init(&sqpi_para);

    id = norflash_read_id();
    if(NORFLASH_ID == id) {
        norflash_quad_enable();
        norflash_gpio_quad_switch();
        id = norflash_read_id();
    }

    return id;
}

/*!
    \brief      enable flash quad operation
    \param[in]  none
    \param[out] none
    \retval     none
*/
void norflash_quad_enable(void)
{
    norflash_write_enable();
    SQPI_INIT = SQPI_ADDR_0BIT;
    sqpi_write_command_config(SQPI_MODE_SSS, 0U, NORFLASH_WRSR);
    *(uint16_t *)SQPI_LOGIC_ADDR = 0x0200U;
    /* wait until the internal WRSR cycle finishes, flash ignores commands while busy */
    norflash_state_check(NORFLASH_WIP_FLAG, 0x00U);
}

/*!
    \brief      read flash identification
    \param[in]  none
    \param[out] none
    \retval     flash identification
*/
uint32_t norflash_read_id(void)
{
    uint32_t temp = 0U;

    SQPI_INIT = SQPI_ADDR_0BIT;
    sqpi_read_command_config(SQPI_MODE_SSS, 0U, NORFLASH_RDID);
    sqpi_read_id_command();
    delay_1ms(1U);
    temp = sqpi_high_id_receive();
    temp = sqpi_low_id_receive();
    return (temp & 0xFFFFFF00U);
}

/*!
    \brief      erase the specified flash sector, 4KB aligned
    \param[in]  sector_addr: address of the sector to erase
    \param[out] none
    \retval     none
*/
void norflash_sector_erase(uint32_t sector_addr)
{
    norflash_write_enable();
    SQPI_INIT = SQPI_ADDR_16BITS;
    sqpi_write_command_config(SQPI_MODE_SSS, 0U, NORFLASH_SE);
    *(uint8_t *)(SQPI_LOGIC_ADDR + ((sector_addr & 0x00FFFF00U) >> 8U)) = (uint8_t)(sector_addr & 0xFFU);

    norflash_state_check(NORFLASH_WIP_FLAG, 0x00U);
}

/*!
    \brief      erase the entire flash
    \param[in]  none
    \param[out] none
    \retval     none
*/
void norflash_chip_erase(void)
{
    norflash_write_enable();

    SQPI_INIT = SQPI_ADDR_0BIT;
    sqpi_write_command_config(SQPI_MODE_SSS, 0U, NORFLASH_CE);
    sqpi_special_command();

    norflash_state_check(NORFLASH_WIP_FLAG, 0x00U);
}

/*!
    \brief      read a block of data from the flash
    \param[in]  read_addr: flash's internal address to read from
    \param[in]  read_buff: pointer to the buffer that receives the data
    \param[in]  size: number of bytes to read
    \param[out] none
    \retval     none
*/
void norflash_read(uint32_t read_addr, uint8_t *read_buff, uint32_t size)
{
    uint32_t i = 0U;
    SQPI_INIT = SQPI_ADDR_24BITS;
    sqpi_read_command_config(SQPI_MODE_SSQ, 8U, NORFLASH_QUADREAD);
    for(i = 0U; i < size; i++) {
        read_buff[i] = *(uint8_t *)(SQPI_LOGIC_ADDR + read_addr + i);
    }
}

/*!
    \brief      write a block of data to the flash, split into 256-byte pages
    \param[in]  write_addr: flash's internal address to write to
    \param[in]  pbuffer: pointer to the buffer
    \param[in]  size: number of bytes to write
    \param[out] none
    \retval     none
*/
void norflash_write(uint32_t write_addr, uint8_t *pbuffer, uint32_t size)
{
    uint32_t count = size;

    while(count > 0U) {
        uint32_t room = NORFLASH_PAGE_SIZE - (write_addr % NORFLASH_PAGE_SIZE);
        uint32_t chunk = (count < room) ? count : room;

        norflash_page_write(write_addr, pbuffer, chunk);
        write_addr += chunk;
        pbuffer += chunk;
        count -= chunk;
    }
}
