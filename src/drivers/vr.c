/*!
    \file    vr.c
    \brief   variable resistor driver for GD32F5HCR-EVAL

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

#include "vr.h"
#include "systick.h"

#define VRn  2U

static const uint8_t vr_channel[VRn] = {ADC_CHANNEL_0, ADC_CHANNEL_5};

/*!
    \brief      initialize ADC pins and peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void vr_init(void)
{
    /* VR1 = PA0, VR2 = PC1 */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0);
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_1);

    rcu_periph_clock_enable(RCU_ADC);
    adc_clock_config(ADC_ADCCK_HCLK_DIV6);
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(ADC_ROUTINE_CHANNEL, 1U);
    adc_external_trigger_config(ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE);

    adc_enable();
    delay_1ms(1U);
}

/*!
    \brief      sample the selected VR
    \param[in]  vr: specify the VR to be sampled
      \arg        VR1
      \arg        VR2
    \param[out] none
    \retval     raw 12-bit ADC value
*/
uint16_t vr_raw_get(vr_t vr)
{
    adc_routine_channel_config(0U, vr_channel[vr], ADC_SAMPLETIME_14POINT5);
    adc_software_trigger_enable(ADC_ROUTINE_CHANNEL);
    while(!adc_flag_get(ADC_FLAG_EORC)) {
    }
    adc_flag_clear(ADC_FLAG_EORC);

    return adc_routine_data_read();
}

/*!
    \brief      sample the selected VR
    \param[in]  vr: specify the VR to be sampled
      \arg        VR1
      \arg        VR2
    \param[out] none
    \retval     voltage in millivolts, 0 ~ 3300
*/
uint32_t vr_millivolt_get(vr_t vr)
{
    return (uint32_t)vr_raw_get(vr) * 3300U / 4095U;
}
