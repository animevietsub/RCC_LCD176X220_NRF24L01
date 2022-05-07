/**
 ******************************************************************************
 * @file           : adc_sensor_library.c
 * @brief          : ADC sensor library
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 Espressif.
 * All rights reserved.
 *
 * Vo Duc Toan / B1907202
 * Can Tho University.
 * March - 2022
 * Built with ESP-IDF Version: 4.4.
 * Target device: ESP32-WROOM.
 *
 ******************************************************************************
 */
#include <stdio.h>
#include "adc_sensor_library.h"
#include "driver/adc.h"

void ADCSensor_Init(void)
{
#if (SENSOR_ADC_UNIT == ADC_UINT_1)
    adc1_config_width(SENSOR_ADC_UNIT);
    adc1_config_channel_atten(SENSOR_LDR_ADC_CHANNEL, SENSOR_ADC_ATTEN);
    adc1_config_channel_atten(SENSOR_RAIN_ADC_CHANNEL, SENSOR_ADC_ATTEN);
#endif
#if (SENSOR_ADC_UNIT == ADC_UINT_2)
    adc2_config_channel_atten(SENSOR_LDR_ADC_CHANNEL, SENSOR_ADC_ATTEN);
    adc2_config_channel_atten(SENSOR_RAIN_ADC_CHANNEL, SENSOR_ADC_ATTEN);
#endif
}

void ADCSensor_GetLDRValue(uint16_t *val)
{
    uint32_t temp = 0;
    for (int i = 0; i < SENSOR_ADC_SAMPLES; i++)
    {
#if (SENSOR_ADC_UNIT == ADC_UINT_1)
        temp += adc1_get_raw(SENSOR_LDR_ADC_CHANNEL);
#endif
#if (SENSOR_ADC_UNIT == ADC_UINT_2)
        int raw;
        adc2_get_raw(SENSOR_LDR_ADC_CHANNEL, SENSOR_ADC_WIDTH, &raw);
        temp += raw;
#endif
    }
    temp /= SENSOR_ADC_SAMPLES;
    *val = temp;
}

void ADCSensor_GetRainValue(uint16_t *val)
{
    uint32_t temp = 0;
    for (int i = 0; i < SENSOR_ADC_SAMPLES; i++)
    {
#if (SENSOR_ADC_UNIT == ADC_UINT_1)
        temp += adc1_get_raw(SENSOR_RAIN_ADC_CHANNEL);
#endif
#if (SENSOR_ADC_UNIT == ADC_UINT_2)
        int raw;
        adc2_get_raw(SENSOR_RAIN_ADC_CHANNEL, SENSOR_ADC_WIDTH, &raw);
        temp += raw;
#endif
    }
    temp /= SENSOR_ADC_SAMPLES;
    *val = temp;
}
