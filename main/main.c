/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : RCC_LCD176X220_NRF24L01
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
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "lcd_com.h"
#include "lcd_lib.h"
#include "fontx.h"
#include "decode_jpeg.h"
#include "animation.h"

#include "ili9225.h"

#include "port.h"
#include "main.h"
#include "fastmath.h"

#include "mirf.h"

#define DRIVER "ST7775"
#define INTERVAL 500
#define WAIT vTaskDelay(INTERVAL)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static const char *TAG_I2S = "[HC595_I2S]";
static const char *TAG_SPIFFS = "[SPIFFS]";
static const char *TAG_JOYSICK = "[JOYSICK]";

static int16_t joyl_left_value = 0;
static int16_t joyl_righ_value = 0;
static int16_t joyl_up_value = 0;
static int16_t joyl_down_value = 0;

static int16_t joyr_left_value = 0;
static int16_t joyr_righ_value = 0;
static int16_t joyr_up_value = 0;
static int16_t joyr_down_value = 0;

static int16_t joyl_x_value = 0;
static int16_t joyl_y_value = 0;
static int16_t joyl_cx_value = 0;
static int16_t joyl_cy_value = 0;
static int16_t joyl_rxl_value = 1;
static int16_t joyl_ryu_value = 1;
static int16_t joyl_rxr_value = 1;
static int16_t joyl_ryd_value = 1;
static int16_t joyl_angle = 0;
static int16_t joyl_mag = 0;

static int16_t joyr_x_value = 0;
static int16_t joyr_y_value = 0;
static int16_t joyr_cx_value = 0;
static int16_t joyr_cy_value = 0;
static int16_t joyr_rxl_value = 1;
static int16_t joyr_ryu_value = 1;
static int16_t joyr_rxr_value = 1;
static int16_t joyr_ryd_value = 1;
static int16_t joyr_angle = 0;
static int16_t joyr_mag = 0;

nvs_handle_t nvs_handle_data;
SemaphoreHandle_t xSemaphore1;
nrf24l01_data_t nrf24l01_data = {
    .AL_DATA = 0,
    .ML_DATA = 0,
    .AR_DATA = 0,
    .MR_DATA = 0,
};
uint8_t *nrf24l01_data_bytes;

void writeStructToByte(const void *object, size_t size, uint8_t *out_bytes)
{
    unsigned char *byte;
    for (byte = object; size--; ++byte)
    {
        *out_bytes = *byte;
        out_bytes++;
    }
}

static void checkSPIFFS(char *path)
{
    DIR *dir = opendir(path);
    assert(dir != NULL);
    while (1)
    {
        struct dirent *data = readdir(dir);
        if (!data)
            break;
        ESP_LOGI(TAG_SPIFFS, "d_name=%s", data->d_name);
    }
    closedir(dir);
}

TickType_t JPEGLOGO(TFT_t *dev, char *file, int width, int height)
{
    TickType_t startTick, endTick, diffTick;
    lcdSetFontDirection(dev, 0);
    lcdFillScreen(dev, BLACK);
    startTick = xTaskGetTickCount();
    pixel_jpeg **pixels;
    uint16_t imageWidth;
    uint16_t imageHeight;
    // uint freeRAM = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    // ESP_LOGI("[DRAM]", "free RAM is %d.", freeRAM);
    esp_err_t err = decode_jpeg(&pixels, file, width, height, &imageWidth, &imageHeight);
    if (err == ESP_OK)
    {
        ESP_LOGI(__FUNCTION__, "imageWidth=%d imageHeight=%d", imageWidth, imageHeight);

        uint16_t jpegWidth = width;
        uint16_t offsetX = 0;
        if (width > imageWidth)
        {
            jpegWidth = imageWidth;
            offsetX = (width - imageWidth) / 2;
        }
        ESP_LOGD(__FUNCTION__, "jpegWidth=%d offsetX=%d", jpegWidth, offsetX);

        uint16_t jpegHeight = height;
        uint16_t offsetY = 0;
        if (height > imageHeight)
        {
            jpegHeight = imageHeight;
            offsetY = (height - imageHeight) / 2;
        }
        ESP_LOGD(__FUNCTION__, "jpegHeight=%d offsetY=%d", jpegHeight, offsetY);
        uint16_t *colors = (uint16_t *)malloc(sizeof(uint16_t) * jpegWidth);
        for (int y = 0; y < jpegHeight; y++)
        {
            for (int x = 0; x < jpegWidth; x++)
            {
                colors[x] = pixels[y][x];
            }
            lcdDrawMultiPixels(dev, offsetX, y + offsetY, jpegWidth, colors);
            // vTaskDelay(1);
        }
        free(colors);
        release_image(&pixels, width, height);
        ESP_LOGD(__FUNCTION__, "Finish");
    }
    else
    {
        ESP_LOGE(__FUNCTION__, "decode_image err=%d imageWidth=%d imageHeight=%d", err, imageWidth, imageHeight);
    }

    endTick = xTaskGetTickCount();
    diffTick = endTick - startTick;
    ESP_LOGI(__FUNCTION__, "elapsed time[ms]:%d", diffTick * portTICK_RATE_MS);
    return diffTick;
}

static void taskLCDContoller()
{
    FontxFile fx16G[2];
    FontxFile fx24G[2];
    FontxFile fx32G[2];
    InitFontx(fx16G, "/spiffs/ILGH16XB.FNT", ""); // 8x16Dot Gothic
    InitFontx(fx24G, "/spiffs/ILGH24XB.FNT", ""); // 12x24Dot Gothic
    InitFontx(fx32G, "/spiffs/ILGH32XB.FNT", ""); // 16x32Dot Gothic
    FontxFile fx16M[2];
    FontxFile fx24M[2];
    FontxFile fx32M[2];
    InitFontx(fx16M, "/spiffs/ILMH16XB.FNT", ""); // 8x16Dot Mincyo
    InitFontx(fx24M, "/spiffs/ILMH24XB.FNT", ""); // 12x24Dot Mincyo
    InitFontx(fx32M, "/spiffs/ILMH32XB.FNT", ""); // 16x32Dot Mincyo
    TFT_t dev;
    lcd_interface_cfg(&dev, 1);
    ili9225_lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);
    // char stats_buffer[1024];
    // vTaskList(stats_buffer);
    // ESP_LOGI("[stats_buffer]", "%s", stats_buffer);
#if CONFIG_INVERSION
    ESP_LOGI(TAG, "Enable Display Inversion");
    lcdInversionOn(&dev);
#endif
    while (1)
    {
        char file[32];
        strcpy(file, "/spiffs/logo_gamo.jpg");
        JPEGLOGO(&dev, file, CONFIG_WIDTH, CONFIG_HEIGHT);

        nvs_get_i16(nvs_handle_data, "joyl_left_value", &joyl_left_value);
        nvs_get_i16(nvs_handle_data, "joyl_righ_value", &joyl_righ_value);
        nvs_get_i16(nvs_handle_data, "joyl_up_value", &joyl_up_value);
        nvs_get_i16(nvs_handle_data, "joyl_down_value", &joyl_down_value);

        nvs_get_i16(nvs_handle_data, "joyr_left_value", &joyr_left_value);
        nvs_get_i16(nvs_handle_data, "joyr_righ_value", &joyr_righ_value);
        nvs_get_i16(nvs_handle_data, "joyr_up_value", &joyr_up_value);
        nvs_get_i16(nvs_handle_data, "joyr_down_value", &joyr_down_value);

        nvs_get_i16(nvs_handle_data, "joyl_cx_value", &joyl_cx_value);
        nvs_get_i16(nvs_handle_data, "joyl_cy_value", &joyl_cy_value);
        nvs_get_i16(nvs_handle_data, "joyr_cx_value", &joyr_cx_value);
        nvs_get_i16(nvs_handle_data, "joyr_cy_value", &joyr_cy_value);

        WAIT;
        if (gpio_get_level(SWITCH_L_PIN) == false && gpio_get_level(SWITCH_R_PIN) == false)
        {
            taskJoyCalibration(&dev, fx16G);
        }

        joyl_rxr_value = (joyl_righ_value - joyl_cx_value) / 100;
        joyl_rxl_value = (joyl_cx_value - joyl_left_value) / 100;
        joyl_ryd_value = (joyl_down_value - joyl_cy_value) / 100;
        joyl_ryu_value = (joyl_cy_value - joyl_up_value) / 100;

        joyr_rxr_value = (joyr_righ_value - joyr_cx_value) / 100;
        joyr_rxl_value = (joyr_cx_value - joyr_left_value) / 100;
        joyr_ryd_value = (joyr_down_value - joyr_cy_value) / 100;
        joyr_ryu_value = (joyr_cy_value - joyr_up_value) / 100;

        strcpy(file, "/spiffs/background.jpg");
        JPEGLOGO(&dev, file, CONFIG_WIDTH, CONFIG_HEIGHT);
        vTaskDelay(10);
        char COMMON_TEXT[32];
        while (1)
        {
            // if (menu_list == MENU_MANUAL_AUTO)
            //     lcdSetFontUnderLine(&dev, RED);
            // lcdSetFontFill(&dev, GREEN);
            // setAutoManualText(&dev, fx16M, AUTO_MANUAL);
            // lcdUnsetFontUnderLine(&dev);
            // lcdUnsetFontFill(&dev);
            setLX(&dev, fx16G, joyl_x_value);
            setLY(&dev, fx16G, joyl_y_value);
            setRX(&dev, fx16G, joyr_x_value);
            setRY(&dev, fx16G, joyr_y_value);
            setAL(&dev, fx16G, joyl_angle);
            setML(&dev, fx16G, joyl_mag);
            setAR(&dev, fx16G, joyr_angle);
            setMR(&dev, fx16G, joyr_mag);
            // setRSV(&dev, fx16G, rain_value);
            // if (menu_list == MENU_LST)
            //     lcdSetFontUnderLine(&dev, RED);
            // setLST(&dev, fx16G, ldr_threshold);
            // lcdUnsetFontUnderLine(&dev);
            // if (menu_list == MENU_RST)
            //     lcdSetFontUnderLine(&dev, RED);
            // setRST(&dev, fx16G, rain_threshold);
            // lcdUnsetFontUnderLine(&dev);
            // switch (weather_status)
            // {
            // case WEATHER_SUNNY:
            //     drawSunny(&dev, 74, 15);
            //     strcpy(COMMON_TEXT, "Sunny");
            //     setWeatherText(&dev, fx16G, COMMON_TEXT);
            //     break;
            // case WEATHER_SUNSHOWER:
            //     drawSunshower(&dev, 74, 15);
            //     strcpy(COMMON_TEXT, "Sunshower");
            //     setWeatherText(&dev, fx16G, COMMON_TEXT);
            //     break;
            // case WEATHER_RAINY:
            //     drawRainy(&dev, 74, 15);
            //     strcpy(COMMON_TEXT, "Rainy");
            //     setWeatherText(&dev, fx16G, COMMON_TEXT);
            //     break;
            // }
            // switch (my_action)
            // {
            // case ACTION_DRYING:
            //     strcpy(COMMON_TEXT, "Drying");
            //     setStatusText(&dev, fx16G, COMMON_TEXT);
            //     break;
            // case ACTION_STORING:
            //     strcpy(COMMON_TEXT, "Storing");
            //     setStatusText(&dev, fx16G, COMMON_TEXT);
            //     break;
            // }
            // strcpy(COMMON_TEXT, "0706825803");
            // setPhoneText(&dev, fx16G, COMMON_TEXT);

            xSemaphoreTake(xSemaphore1, pdMS_TO_TICKS(500));
            // setSV(&dev, fx16G, (uint8_t)random() % 100);
            // setCV(&dev, fx16G, distance);
            // setP(&dev, fx16G, (uint8_t)random() % 100);
            // setI(&dev, fx16G, (uint8_t)random() % 100);
            // setD(&dev, fx16G, (uint8_t)random() % 100);
            // drawLightRED(&dev, 94, 78);
            // setDisplaySpeed(&dev, (uint8_t)random() % 100);
            // setDisplayLevel(&dev, MIN(500, MAX(250, distance)) * (-4) / 10 + 200);
            // vTaskDelay(pdMS_TO_TICKS(200));
            // drawLightGREEN(&dev, 94, 78);

            // setDisplaySpeed(&dev, (uint8_t)random() % 100);
            // setDisplayLevel(&dev, (uint8_t)random() % 100);
            // lcdDrawFillRect(&dev, 15, 188, 144, 197, BLACK);
            // vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
    vTaskDelete(NULL);
}

void taskGetADC(adc1_channel_t channel, int16_t *val)
{
    int32_t temp = 0;
    for (int i = 0; i < JOYSTICK_ADC_SAMPLES; i++)
    {
        temp += adc1_get_raw(channel);
    }
    temp /= JOYSTICK_ADC_SAMPLES;
    *val = temp;
}

void taskJoyCalibration(TFT_t *dev, FontxFile *fx)
{
    lcdFillScreen(dev, BLACK);
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char ascii[20];
    strcpy(ascii, "LEFT");
    setTextInCenter(dev, fx, ascii, 10, YELLOW);
    for (int8_t i = 5; i > 0; i--)
    {
        sprintf(ascii, "%d ...", i);
        setTextInCenter(dev, fx, ascii, -10, RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    taskGetADC(JOYSTICK_LX_ADC_CHANNEL, &joyl_left_value);
    taskGetADC(JOYSTICK_RX_ADC_CHANNEL, &joyr_left_value);

    lcdFillScreen(dev, BLACK);
    strcpy(ascii, "RIGHT");
    setTextInCenter(dev, fx, ascii, 10, YELLOW);
    for (int8_t i = 5; i > 0; i--)
    {
        sprintf(ascii, "%d ...", i);
        setTextInCenter(dev, fx, ascii, -10, RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    taskGetADC(JOYSTICK_LX_ADC_CHANNEL, &joyl_righ_value);
    taskGetADC(JOYSTICK_RX_ADC_CHANNEL, &joyr_righ_value);

    lcdFillScreen(dev, BLACK);
    strcpy(ascii, "UP");
    setTextInCenter(dev, fx, ascii, 10, YELLOW);
    for (int8_t i = 5; i > 0; i--)
    {
        sprintf(ascii, "%d ...", i);
        setTextInCenter(dev, fx, ascii, -10, RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    taskGetADC(JOYSTICK_LY_ADC_CHANNEL, &joyl_up_value);
    taskGetADC(JOYSTICK_RY_ADC_CHANNEL, &joyr_up_value);

    lcdFillScreen(dev, BLACK);
    strcpy(ascii, "DOWN");
    setTextInCenter(dev, fx, ascii, 10, YELLOW);
    for (int8_t i = 5; i > 0; i--)
    {
        sprintf(ascii, "%d ...", i);
        setTextInCenter(dev, fx, ascii, -10, RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    taskGetADC(JOYSTICK_LY_ADC_CHANNEL, &joyl_down_value);
    taskGetADC(JOYSTICK_RY_ADC_CHANNEL, &joyr_down_value);

    lcdFillScreen(dev, BLACK);
    strcpy(ascii, "CENTER");
    setTextInCenter(dev, fx, ascii, 10, YELLOW);
    for (int8_t i = 5; i > 0; i--)
    {
        sprintf(ascii, "%d ...", i);
        setTextInCenter(dev, fx, ascii, -10, RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    taskGetADC(JOYSTICK_LX_ADC_CHANNEL, &joyl_cx_value);
    taskGetADC(JOYSTICK_LY_ADC_CHANNEL, &joyl_cy_value);
    taskGetADC(JOYSTICK_RX_ADC_CHANNEL, &joyr_cx_value);
    taskGetADC(JOYSTICK_RY_ADC_CHANNEL, &joyr_cy_value);

    nvs_set_i16(nvs_handle_data, "joyl_righ_value", joyl_righ_value);
    nvs_set_i16(nvs_handle_data, "joyr_righ_value", joyr_righ_value);
    nvs_set_i16(nvs_handle_data, "joyl_left_value", joyl_left_value);
    nvs_set_i16(nvs_handle_data, "joyr_left_value", joyr_left_value);
    nvs_set_i16(nvs_handle_data, "joyl_down_value", joyl_down_value);
    nvs_set_i16(nvs_handle_data, "joyr_down_value", joyr_down_value);
    nvs_set_i16(nvs_handle_data, "joyl_up_value", joyl_up_value);
    nvs_set_i16(nvs_handle_data, "joyr_up_value", joyr_up_value);

    nvs_set_i16(nvs_handle_data, "joyl_cx_value", joyl_cx_value);
    nvs_set_i16(nvs_handle_data, "joyl_cy_value", joyl_cy_value);
    nvs_set_i16(nvs_handle_data, "joyr_cx_value", joyr_cx_value);
    nvs_set_i16(nvs_handle_data, "joyr_cy_value", joyr_cy_value);

    vTaskDelay(pdMS_TO_TICKS(500));
}

void joyStickInit()
{
    adc1_config_width(JOYSTICK_ADC_WIDTH);
    adc1_config_channel_atten(JOYSTICK_LX_ADC_CHANNEL, JOYSTICK_ADC_ATTEN);
    adc1_config_channel_atten(JOYSTICK_LY_ADC_CHANNEL, JOYSTICK_ADC_ATTEN);
    adc1_config_channel_atten(JOYSTICK_RX_ADC_CHANNEL, JOYSTICK_ADC_ATTEN);
    adc1_config_channel_atten(JOYSTICK_RY_ADC_CHANNEL, JOYSTICK_ADC_ATTEN);

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = false,
        .pull_up_en = false,
    };
    io_conf.pin_bit_mask = (1ULL << SWITCH_L_PIN) | (1ULL << SWITCH_R_PIN);
    gpio_config(&io_conf);
}

static void taskJoyStickContoller()
{
    int16_t templ_x, templ_y, tempr_x, tempr_y;
    int16_t joyl_rx_value = 1, joyl_ry_value = 1, joyr_rx_value = 1, joyr_ry_value = 1;
    while (1)
    {
        taskGetADC(JOYSTICK_LX_ADC_CHANNEL, &templ_x);
        taskGetADC(JOYSTICK_LY_ADC_CHANNEL, &templ_y);
        taskGetADC(JOYSTICK_RX_ADC_CHANNEL, &tempr_x);
        taskGetADC(JOYSTICK_RY_ADC_CHANNEL, &tempr_y);

        if (joyl_cx_value - templ_x > 0)
            joyl_rx_value = joyl_rxl_value;
        else
            joyl_rx_value = joyl_rxr_value;
        if (joyl_cy_value - templ_y > 0)
            joyl_ry_value = joyl_ryu_value;
        else
            joyl_ry_value = joyl_ryd_value;

        if (joyr_cx_value - tempr_x > 0)
            joyr_rx_value = joyr_rxl_value;
        else
            joyr_rx_value = joyr_rxr_value;
        if (joyr_cy_value - tempr_y > 0)
            joyr_ry_value = joyr_ryu_value;
        else
            joyr_ry_value = joyr_ryd_value;

        joyl_x_value = -(joyl_cx_value - templ_x) / (joyl_rx_value);
        joyl_y_value = (joyl_cy_value - templ_y) / (joyl_ry_value);
        joyr_x_value = -(joyr_cx_value - tempr_x) / (joyr_rx_value);
        joyr_y_value = (joyr_cy_value - tempr_y) / (joyr_ry_value);
        joyl_mag = (int16_t)sqrtf(joyl_x_value * joyl_x_value + joyl_y_value * joyl_y_value);
        if (joyl_cx_value - templ_x > 0)
            joyl_angle = -(int16_t)(acosf((float)joyl_y_value / joyl_mag) * RAD_TO_DEGREE);
        else
            joyl_angle = (int16_t)(acosf((float)joyl_y_value / joyl_mag) * RAD_TO_DEGREE);
        joyr_mag = (int16_t)sqrtf(joyr_x_value * joyr_x_value + joyr_y_value * joyr_y_value);
        if (joyr_cx_value - tempr_x > 0)
            joyr_angle = -(int16_t)(acosf((float)joyr_y_value / joyr_mag) * RAD_TO_DEGREE);
        else
            joyr_angle = (int16_t)(acosf((float)joyr_y_value / joyr_mag) * RAD_TO_DEGREE);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}

static void taskNRFTransmitter()
{
    NRF24_t dev;
    Nrf24_init(&dev);
    uint8_t payload = sizeof(nrf24l01_data_t);
    uint8_t channel = 90;
    Nrf24_config(&dev, channel, payload);
    Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
    Nrf24_SetSpeedDataRates(&dev, 1);
    Nrf24_setRetransmitDelay(&dev, 0);
    Nrf24_printDetails(&dev);
    while (1)
    {
        nrf24l01_data.AL_DATA = joyl_angle;
        nrf24l01_data.ML_DATA = joyl_mag;
        nrf24l01_data.AR_DATA = joyr_angle;
        nrf24l01_data.MR_DATA = joyr_mag;
        writeStructToByte(&nrf24l01_data, sizeof(nrf24l01_data_t), nrf24l01_data_bytes);
        Nrf24_send(&dev, nrf24l01_data_bytes);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    xSemaphore1 = xSemaphoreCreateBinary();
    nrf24l01_data_bytes = malloc(sizeof(nrf24l01_data_t));
    ESP_LOGI(TAG_SPIFFS, "Initializing SPIFFS");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 10,
        .format_if_mount_failed = true,
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG_SPIFFS, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG_SPIFFS, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG_SPIFFS, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_SPIFFS, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG_SPIFFS, "Partition size: total: %d, used: %d", total, used);
    }
    // esp_spiffs_format(conf.partition_label);
    checkSPIFFS("/spiffs/"); // Check files
    ESP_LOGI(TAG_I2S, "Starting init LCD_I2S");
    HC595_I2SInit();
    nvs_flash_init();
    nvs_open("storage", NVS_READWRITE, &nvs_handle_data);
    joyStickInit();
    xTaskCreate(taskLCDContoller, "[taskLCDContoller]", 1024 * 6, NULL, 2, NULL);
    xTaskCreate(taskJoyStickContoller, "[taskJoyStickContoller]", 1024 * 6, NULL, 3, NULL);
    xTaskCreate(taskNRFTransmitter, "[taskNRFTransmitter]", 1024 * 3, NULL, 2, NULL);
    // xTaskCreate(taskMotorController, "[taskMotorController]", 1024 * 3, NULL, 2, NULL);
    // xTaskCreate(taskADCSensor, "[taskADCSensor]", 1024 * 3, NULL, 2, NULL);
    // xTaskCreate(taskButton, "[taskButton]", 1024 * 3, NULL, 2, NULL);
    // xTaskCreate(taskSpeedControl, "[taskSpeedControl]", 1024 * 3, NULL, 2, NULL);
}
