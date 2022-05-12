/**
 ******************************************************************************
 * @file           : animation.c
 * @brief          : WLC_LCD176X220_PID
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
#include "animation.h"

void setLX(TFT_t *dev, FontxFile *fx, int16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[7];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 135, 48, 135 + fontHeight, 48 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 135, 48, text, GREEN);
}

void setLY(TFT_t *dev, FontxFile *fx, int16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[7];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 118, 48, 118 + fontHeight, 48 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 118, 48, text, GREEN);
}

void setRX(TFT_t *dev, FontxFile *fx, int16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[7];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 101, 48, 101 + fontHeight, 48 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 101, 48, text, CYAN);
}

void setRY(TFT_t *dev, FontxFile *fx, int16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[7];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 84, 48, 84 + fontHeight, 48 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 84, 48, text, CYAN);
}

void setAL(TFT_t *dev, FontxFile *fx, int16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[7];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 68, 48, 68 + fontHeight, 48 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 68, 48, text, RED);
}

void setML(TFT_t *dev, FontxFile *fx, int16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[7];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 51, 48, 51 + fontHeight, 48 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 51, 48, text, RED);
}

void setAR(TFT_t *dev, FontxFile *fx, int16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[7];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 34, 48, 34 + fontHeight, 48 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 34, 48, text, 0xF999);
}

void setMR(TFT_t *dev, FontxFile *fx, int16_t value)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    char text[7];
    sprintf(text, "%d", value);
    lcdDrawFillRect(dev, 17, 48, 17 + fontHeight, 48 + fontWidth * 5, BLACK);
    lcdDrawString(dev, fx, 17, 48, text, 0xF999);
}

void drawImage(TFT_t *dev, char *file, uint16_t offsetX, uint16_t offsetY, uint16_t width, uint16_t height)
{
    pixel_jpeg **pixels;
    uint16_t imageWidth;
    uint16_t imageHeight;
    esp_err_t err = decode_jpeg(&pixels, file, width, height, &imageWidth, &imageHeight);
    if (err == ESP_OK)
    {
        uint16_t *colors = (uint16_t *)malloc(sizeof(uint16_t) * width);
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                colors[x] = pixels[y][x];
            }
            lcdDrawMultiPixels(dev, offsetX, y + offsetY, width, colors);
        }
        free(colors);
        release_image(&pixels, width, height);
    }
    else
    {
        ESP_LOGE(__FUNCTION__, "decode_image err=%d imageWidth=%d imageHeight=%d", err, imageWidth, imageHeight);
    }
}

void setTextInCenter(TFT_t *dev, FontxFile *fx, char *text, int8_t offset, uint16_t color)
{
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    lcdSetFontDirection(dev, 1);
    lcdDrawFillRect(dev, CONFIG_WIDTH / 2 + offset, CONFIG_HEIGHT / 2 - fontWidth * strlen(text) / 2, CONFIG_WIDTH / 2 + offset + fontHeight, CONFIG_HEIGHT / 2 + fontWidth * (-strlen(text) / 2 + 12), BLACK);
    lcdDrawString(dev, fx, CONFIG_WIDTH / 2 + offset, CONFIG_HEIGHT / 2 - fontWidth * strlen(text) / 2, text, color);
}
