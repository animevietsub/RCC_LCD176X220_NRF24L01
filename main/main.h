/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : RAIN_SENSOR_WITH_SIM800L
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
#ifndef __MAIN_H__
#define __MAIN_H__

#include "soc/adc_channel.h"

#define JOYSTICK_ADC_WIDTH ADC_WIDTH_BIT_12
#define JOYSTICK_ADC_ATTEN ADC_ATTEN_DB_0
#define JOYSTICK_ADC_UNIT ADC_UNIT_1
#define JOYSTICK_ADC_SAMPLES 32

#define JOYSTICK_LX_ADC_CHANNEL ADC1_GPIO35_CHANNEL
#define JOYSTICK_LY_ADC_CHANNEL ADC1_GPIO32_CHANNEL
#define JOYSTICK_RX_ADC_CHANNEL ADC1_GPIO34_CHANNEL
#define JOYSTICK_RY_ADC_CHANNEL ADC1_GPIO39_CHANNEL

#define JOYSTICK_LB_PINOUT 

typedef enum
{
    WEATHER_SUNNY,
    WEATHER_RAINY,
    WEATHER_SUNSHOWER,
} weather_status_t;

typedef enum
{
    DUMMY = -1,
    MENU_IDLE = 0,
    MENU_MANUAL_AUTO,
    MENU_LST,
    MENU_RST,
} menu_list_t;

typedef enum
{
    ACTION_DRYING = 0,
    ACTION_STORING,
} my_action_t;

void taskJoyCalibration(TFT_t *dev, FontxFile *fx);
void taskGetADC(adc1_channel_t channel, uint16_t *val);

#endif
