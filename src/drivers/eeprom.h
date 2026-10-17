/*!
    \file    eeprom.h
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

#ifndef EEPROM_H
#define EEPROM_H

#include "gd32w51x_f5hc.h"

/* AT24C02C on I2C1: SCL = PB15, SDA = PA8, 400kHz, 8-byte page */
#define EEPROM_I2C_ADDR       0xA0U
#define EEPROM_SIZE           256U
#define EEPROM_PAGE_SIZE      8U

/* function declarations */
/* initialize I2C1 peripheral used by the EEPROM */
void eeprom_init(void);
/* read a block of data from the EEPROM */
ErrStatus eeprom_read(uint8_t *p_buffer, uint16_t read_addr, uint16_t number_of_byte);
/* write a block of data to the EEPROM, split into page cycles automatically */
ErrStatus eeprom_write(uint8_t *p_buffer, uint16_t write_addr, uint16_t number_of_byte);

#endif /* EEPROM_H */
