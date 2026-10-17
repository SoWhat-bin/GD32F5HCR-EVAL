/*!
    \file    eeprom.c
    \brief   eeprom driver for AT24C02C on GD32F5HCR-EVAL

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

#include "eeprom.h"
#include "systick.h"
#include <stddef.h>

#define I2C_SPEED             400000U
#define I2C_TIME_OUT          (uint32_t)(20000)
#define I2C_MAX_RELOAD_SIZE   255U

typedef enum {
    I2C_START = 0,
    I2C_SEND_ADDRESS,
    I2C_RESTART,
    I2C_TRANSMIT_DATA,
    I2C_RELOAD,
    I2C_STOP,
    I2C_END
} i2c_process_enum;

/*!
    \brief      configure GPIO ports of I2C1
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void eeprom_gpio_config(void)
{
    /* SCL = PB15, SDA = PA8 */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_I2C1);

    gpio_af_set(GPIOB, GPIO_AF_6, GPIO_PIN_15);
    gpio_af_set(GPIOA, GPIO_AF_6, GPIO_PIN_8);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_15);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_8);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO_PIN_8);
}

/*!
    \brief      configure I2C1 to 400kHz
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void eeprom_i2c_config(void)
{
    i2c_timing_config(I2C1, 0, 0x4, 0);
    i2c_master_clock_config(I2C1, 0x1A, 0x4F);
    i2c_enable(I2C1);
}

/*!
    \brief      reset I2C1 bus by toggling SCL/SDA as GPIO
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void eeprom_i2c_bus_reset(void)
{
    /* configure SCL/SDA for GPIO */
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_15);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_8);

    GPIO_BC(GPIOB) |= GPIO_PIN_15;
    GPIO_BC(GPIOA) |= GPIO_PIN_8;
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    GPIO_BOP(GPIOB) |= GPIO_PIN_15;
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    GPIO_BOP(GPIOA) |= GPIO_PIN_8;

    gpio_af_set(GPIOB, GPIO_AF_6, GPIO_PIN_15);
    gpio_af_set(GPIOA, GPIO_AF_6, GPIO_PIN_8);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO_PIN_15);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO_PIN_8);
}

/*!
    \brief      initialize I2C1 peripheral used by the EEPROM
    \param[in]  none
    \param[out] none
    \retval     none
*/
void eeprom_init(void)
{
    eeprom_gpio_config();
    eeprom_i2c_config();
}

/*!
    \brief      write a page of data to the EEPROM with a single write cycle
    \param[in]  p_buffer: pointer to the buffer containing the data to be written
    \param[in]  write_address: EEPROM's internal address to write to
    \param[in]  number_of_byte: number of bytes to write, no more than 8
    \param[out] none
    \retval     SUCCESS or ERROR
*/
static ErrStatus eeprom_page_write(uint8_t *p_buffer, uint16_t write_address, uint16_t number_of_byte)
{
    i2c_process_enum state = I2C_START;
    uint32_t timeout = 0;
    uint8_t end_flag = 0;
    ErrStatus ret = SUCCESS;
    uint16_t count = number_of_byte;

    while((!end_flag) && (SUCCESS == ret)) {
        switch(state) {
        case I2C_START:
            i2c_master_addressing(I2C1, EEPROM_I2C_ADDR, I2C_MASTER_TRANSMIT);
            i2c_transfer_byte_number_config(I2C1, number_of_byte + 1U);
            I2C_STAT(I2C1) |= I2C_STAT_TBE;
            i2c_automatic_end_enable(I2C1);
            while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_start_on_bus(I2C1);
                timeout = 0;
                state = I2C_SEND_ADDRESS;
            } else {
                eeprom_i2c_bus_reset();
                timeout = 0;
                state = I2C_START;
                ret = ERROR;
            }
            break;
        case I2C_SEND_ADDRESS:
            while((!i2c_flag_get(I2C1, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_data_transmit(I2C1, (uint8_t)write_address);
                timeout = 0;
                state = I2C_TRANSMIT_DATA;
            } else {
                timeout = 0;
                state = I2C_START;
                ret = ERROR;
            }
            break;
        case I2C_TRANSMIT_DATA:
            while(count--) {
                while((!i2c_flag_get(I2C1, I2C_FLAG_TI)) && (timeout < I2C_TIME_OUT)) {
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT) {
                    i2c_data_transmit(I2C1, *p_buffer);
                    p_buffer++;
                    timeout = 0;
                    state = I2C_STOP;
                } else {
                    timeout = 0;
                    state = I2C_START;
                    ret = ERROR;
                    break;
                }
            }
            break;
        case I2C_STOP:
            while((!i2c_flag_get(I2C1, I2C_FLAG_STPDET)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_flag_clear(I2C1, I2C_FLAG_STPDET);
                timeout = 0;
                state = I2C_END;
                end_flag = 1;
            } else {
                timeout = 0;
                state = I2C_START;
                ret = ERROR;
            }
            break;
        default:
            state = I2C_START;
            end_flag = 1;
            timeout = 0;
            ret = ERROR;
            break;
        }
    }

    return ret;
}

/*!
    \brief      read a block of data from the EEPROM
    \param[in]  p_buffer: pointer to the buffer that receives the data
    \param[in]  read_addr: EEPROM's internal address to start reading from
    \param[in]  number_of_byte: number of bytes to read
    \param[out] none
    \retval     SUCCESS or ERROR
*/
ErrStatus eeprom_read(uint8_t *p_buffer, uint16_t read_addr, uint16_t number_of_byte)
{
    uint32_t nbytes_reload = 0U;
    i2c_process_enum state = I2C_START;
    uint32_t timeout = 0;
    uint8_t end_flag = 0;
    uint8_t restart_flag = 0;
    uint8_t first_reload_flag = 1U;
    uint8_t last_reload_flag = 0U;
    ErrStatus ret = SUCCESS;
    uint16_t remain = number_of_byte;

    while((!end_flag) && (SUCCESS == ret)) {
        switch(state) {
        case I2C_START:
            if(0U == restart_flag) {
                I2C_STAT(I2C1) |= I2C_STAT_TBE;
                i2c_master_addressing(I2C1, EEPROM_I2C_ADDR, I2C_MASTER_TRANSMIT);
                i2c_transfer_byte_number_config(I2C1, 1);
                i2c_automatic_end_disable(I2C1);
                while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY) && (timeout < I2C_TIME_OUT)) {
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT) {
                    i2c_start_on_bus(I2C1);
                    timeout = 0;
                    state = I2C_SEND_ADDRESS;
                } else {
                    eeprom_i2c_bus_reset();
                    timeout = 0;
                    state = I2C_START;
                    ret = ERROR;
                }
            } else {
                i2c_start_on_bus(I2C1);
                restart_flag = 0;
                state = I2C_TRANSMIT_DATA;
            }
            break;
        case I2C_SEND_ADDRESS:
            while((!i2c_flag_get(I2C1, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_data_transmit(I2C1, (uint8_t)read_addr);
                timeout = 0;
                state = I2C_RESTART;
            } else {
                timeout = 0;
                state = I2C_START;
                ret = ERROR;
            }
            break;
        case I2C_RESTART:
            while((!i2c_flag_get(I2C1, I2C_FLAG_TC)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_master_addressing(I2C1, EEPROM_I2C_ADDR, I2C_MASTER_RECEIVE);
                i2c_reload_enable(I2C1);
                timeout = 0;
                state = I2C_RELOAD;
                restart_flag = 1;
            } else {
                timeout = 0;
                state = I2C_START;
                ret = ERROR;
            }
            break;
        case I2C_RELOAD:
            if(remain > I2C_MAX_RELOAD_SIZE) {
                remain = remain - I2C_MAX_RELOAD_SIZE;
                nbytes_reload = I2C_MAX_RELOAD_SIZE;
            } else {
                nbytes_reload = remain;
                last_reload_flag = 1U;
            }
            if(1U == first_reload_flag) {
                i2c_transfer_byte_number_config(I2C1, nbytes_reload);
                if(1U == last_reload_flag) {
                    last_reload_flag = 0U;
                    if(remain <= I2C_MAX_RELOAD_SIZE) {
                        i2c_reload_disable(I2C1);
                        i2c_automatic_end_enable(I2C1);
                    }
                }
                first_reload_flag = 0U;
                state = I2C_START;
            } else {
                while((!i2c_flag_get(I2C1, I2C_FLAG_TCR)) && (timeout < I2C_TIME_OUT)) {
                    timeout++;
                }
                if(timeout < I2C_TIME_OUT) {
                    i2c_transfer_byte_number_config(I2C1, nbytes_reload);
                    if(remain <= I2C_MAX_RELOAD_SIZE) {
                        i2c_reload_disable(I2C1);
                        i2c_automatic_end_enable(I2C1);
                    }
                    timeout = 0;
                    state = I2C_TRANSMIT_DATA;
                } else {
                    timeout = 0;
                    state = I2C_START;
                    ret = ERROR;
                }
            }
            break;
        case I2C_TRANSMIT_DATA:
            while((!i2c_flag_get(I2C1, I2C_FLAG_TBE)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                while(nbytes_reload) {
                    if(i2c_flag_get(I2C1, I2C_FLAG_RBNE)) {
                        *p_buffer = i2c_data_receive(I2C1);
                        p_buffer++;
                        nbytes_reload--;
                    }
                }
                timeout = 0;
                if(I2C_CTL1(I2C1) & I2C_CTL1_RELOAD) {
                    timeout = 0;
                    state = I2C_RELOAD;
                } else {
                    timeout = 0;
                    state = I2C_STOP;
                }
            } else {
                timeout = 0;
                state = I2C_START;
                ret = ERROR;
            }
            break;
        case I2C_STOP:
            while((!i2c_flag_get(I2C1, I2C_FLAG_STPDET)) && (timeout < I2C_TIME_OUT)) {
                timeout++;
            }
            if(timeout < I2C_TIME_OUT) {
                i2c_flag_clear(I2C1, I2C_FLAG_STPDET);
                timeout = 0;
                state = I2C_END;
                end_flag = 1;
            } else {
                timeout = 0;
                state = I2C_START;
                ret = ERROR;
            }
            break;
        default:
            state = I2C_START;
            end_flag = 1;
            timeout = 0;
            ret = ERROR;
            break;
        }
    }

    return ret;
}

/*!
    \brief      write a block of data to the EEPROM, split into page cycles
    \param[in]  p_buffer: pointer to the buffer containing the data to be written
    \param[in]  write_addr: EEPROM's internal address to write to
    \param[in]  number_of_byte: number of bytes to write
    \param[out] none
    \retval     SUCCESS or ERROR
*/
ErrStatus eeprom_write(uint8_t *p_buffer, uint16_t write_addr, uint16_t number_of_byte)
{
    uint16_t address = write_addr & 0xFFU;
    uint32_t count = number_of_byte;
    ErrStatus ret = SUCCESS;

    if((NULL == p_buffer) || (0U == number_of_byte) ||
       ((uint32_t)address + number_of_byte > EEPROM_SIZE)) {
        return ERROR;
    }

    while((count > 0U) && (SUCCESS == ret)) {
        uint16_t room = EEPROM_PAGE_SIZE - (address % EEPROM_PAGE_SIZE);
        uint16_t chunk = (count < room) ? (uint16_t)count : room;

        ret = eeprom_page_write(p_buffer, address, chunk);
        /* wait for the internal write cycle to finish */
        delay_1ms(5);
        address += chunk;
        p_buffer += chunk;
        count -= chunk;
    }

    return ret;
}
