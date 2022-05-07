#ifndef __ADC_SENSOR_LIBRARY_H__
#define __ADC_SENSOR_LIBRARY_H__

#include "soc/adc_channel.h"

#define SENSOR_ADC_WIDTH ADC_WIDTH_BIT_12
#define SENSOR_ADC_ATTEN ADC_ATTEN_DB_0
#define SENSOR_ADC_UNIT ADC_UNIT_2
#define SENSOR_ADC_SAMPLES 32

#define SENSOR_LDR_ADC_CHANNEL ADC2_GPIO15_CHANNEL
#define SENSOR_RAIN_ADC_CHANNEL ADC2_GPIO4_CHANNEL

void ADCSensor_Init(void);
void ADCSensor_GetLDRValue(uint16_t *val);
void ADCSensor_GetRainValue(uint16_t *val);

#endif