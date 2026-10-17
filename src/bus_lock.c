/*!
    \file    bus_lock.c
    \brief   mutual exclusion for the shared i2c1 and sqpi buses

    \version 2026-10-19, V1.0.0
*/

/*
    Copyright (c) 2026, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
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

#include "bus_lock.h"

#include "FreeRTOS.h"
#include "semphr.h"

/* the driver self tests and the ui self tests execute in separate tasks on
   this single core: without arbitration they tear each other's multibyte
   i2c1 and sqpi transfers apart, eeprom writes end in a bus timeout and the
   flash readback diverges mid page */
static SemaphoreHandle_t bus_mutex;

void bus_lock_init(void)
{
    bus_mutex = xSemaphoreCreateMutex();
}

bool bus_lock_acquire(uint32_t timeout_ms)
{
    if (NULL == bus_mutex) {
        return false;
    }

    return (pdTRUE == xSemaphoreTake(bus_mutex, pdMS_TO_TICKS(timeout_ms)));
}

void bus_lock_release(void)
{
    if (NULL != bus_mutex) {
        (void)xSemaphoreGive(bus_mutex);
    }
}
