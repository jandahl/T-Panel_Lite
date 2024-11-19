/*
 * @Description: None
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-07-25 13:45:02
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2023-11-22 18:24:00
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"
// #include "material_24Bit.h"
#include "material_16Bit.h"

Arduino_DataBus *bus = new Arduino_SWSPI(-1 /* DC */, LCD_CS /* CS */,
                                         SCLK /* SCK */, MOSI /* MOSI */, -1 /* MISO */);
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    LCD_DE /* DE */, LCD_VSYNC /* VSYNC */, LCD_HSYNC /* HSYNC */, LCD_PCLK /* PCLK */,
    LCD_B0 /* B0 */, LCD_B1 /* B1 */, LCD_B2 /* B2 */, LCD_B3 /* B3 */, LCD_B4 /* B4 */,
    LCD_G0 /* G0 */, LCD_G1 /* G1 */, LCD_G2 /* G2 */, LCD_G3 /* G3 */, LCD_G4 /* G4 */, LCD_G5 /* G5 */,
    LCD_R0 /* R0 */, LCD_R1 /* R1 */, LCD_R2 /* R2 */, LCD_R3 /* R3 */, LCD_R4 /* R4 */,
    1 /* hsync_polarity */, 20 /* hsync_front_porch */, 1 /* hsync_pulse_width */, 1 /* hsync_back_porch */,
    1 /* vsync_polarity */, 30 /* vsync_front_porch */, 1 /* vsync_pulse_width */, 10 /* vsync_back_porch */,
    10 /* pclk_active_neg */, 6000000L /* prefer_speed */, false /* useBigEndian */,
    0 /* de_idle_high*/, 0 /* pclk_idle_high */);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    LCD_WIDTH /* width */, LCD_HEIGHT /* height */, rgbpanel, 0 /* rotation */, true /* auto_flush */,
    bus, -1 /* RST */, st7701_type9_init_operations, sizeof(st7701_type9_init_operations));

void setup(void)
{
    Serial.begin(115200);

    pinMode(33, OUTPUT);
    digitalWrite(33, HIGH);

    gfx->begin();
    gfx->fillScreen(WHITE);

    // gfx->draw24bitRGBBitmap(0, 0, gImage_1, 480, 480); // RGB
    // delay(1000);
    // gfx->draw24bitRGBBitmap(0, 0, gImage_2, 480, 480); // RGB
    // delay(1000);

    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_1, 480, 480); // RGB
    delay(1000);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_2, 480, 480); // RGB
    delay(1000);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_3, 480, 480); // RGB
    delay(1000);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_4, 480, 480); // RGB
    delay(1000);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_5, 480, 480); // RGB
    delay(1000);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_3, 480, 480); // RGB
    delay(1000);
}

void loop()
{
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_1, 480, 480); // RGB
    delay(2000);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_2, 480, 480); // RGB
    delay(2000);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_3, 480, 480); // RGB
    delay(2000);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_4, 480, 480); // RGB
    delay(2000);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_5, 480, 480); // RGB
    delay(2000);

    // gfx->fillScreen(RED);
    // delay(1000);
    // gfx->fillScreen(GREEN);
    // delay(1000);
    // gfx->fillScreen(BLUE);
    // delay(1000);

    // gfx->XL_digitalWrite(4, 0);
    // delay(1000);
    // gfx->XL_digitalWrite(4, 1);
    // delay(1000);
}