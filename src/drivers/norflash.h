/*!
    \file    norflash.h
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
ARISING IN ANY WAY OUT OF THE USE OF THE SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#ifndef NORFLASH_H
#define NORFLASH_H

#include "gd32w51x_f5hc.h"

/* GD25Q16 on SQPI: SCK = PA9, CS = PA10, IO0 = PA11, IO1 = PA12, IO2 = PB3, IO3 = PB4 */
#define NORFLASH_ID              0xC8401500U
#define NORFLASH_PAGE_SIZE       256U
#define NORFLASH_SECTOR_SIZE     4096U
#define NORFLASH_SIZE            (2U * 1024U * 1024U)

/* function declarations */
/* initialize SQPI and the flash, quad mode is enabled on ID match */
uint32_t norflash_init(void);
/* enable flash quad operation */
void norflash_quad_enable(void);
/* read flash identification */
uint32_t norflash_read_id(void);
/* erase the specified 4KB flash sector, address is 4KB aligned */
void norflash_sector_erase(uint32_t sector_addr);
/* erase the entire flash */
void norflash_chip_erase(void);
/* read a block of data from the flash */
void norflash_read(uint32_t read_addr, uint8_t *read_buff, uint32_t size);
/* write a block of data to the flash, split into 256-byte pages */
void norflash_write(uint32_t write_addr, uint8_t *pbuffer, uint32_t size);

#endif /* NORFLASH_H */
