/*
 * @Description: T-Panel-Lite出厂测试程序
 * @Author: LILYGO_L
 * @Date: 2023-09-19 14:36:58
 * @LastEditTime: 2024-07-20 15:57:48
 * @License: GPL 3.0
 */
// #define TOUCH_MODULES_GT911
// #define TOUCH_MODULES_CST_SELF
#define TOUCH_MODULES_CST_MUTUAL
// #define TOUCH_MODULES_ZTW622
// #define TOUCH_MODULES_L58
// #define TOUCH_MODULES_FT3267
// #define TOUCH_MODULES_FT5x06

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "TouchLib.h"
#include "material_16bit.h"
#include "pin_config.h"
#include "WiFi.h"
#include <HTTPClient.h>

#define WIFI_SSID "xinyuandianzi"
#define WIFI_PASSWORD "AA15994823428"
// #define WIFI_SSID "LilyGo-AABB"
// #define WIFI_PASSWORD "xinyuandianzi"

#define WIFI_CONNECT_WAIT_MAX (5000)

#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define GMT_OFFSET_SEC 8 * 3600 // Time zone setting function, written as 8 * 3600 in East Eighth Zone (UTC/GMT+8:00)
#define DAY_LIGHT_OFFSET_SEC 0  // Fill in 3600 for daylight saving time, otherwise fill in 0

bool Get_int = 0;
int Image_Flag = 0;
bool Touch_Flag = 0;
uint8_t Touch_Num = 0;

uint8_t Last_X = 0;
uint8_t Last_Y = 0;

bool Display_Start = 0;

bool wifi_connection_failure_flag = false;
uint32_t wifi_time_delay = 0;

TouchLib touch(Wire, IIC_SDA, IIC_SCL, CST3240_Address);

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

void scan_i2c_device(TwoWire &i2c)
{
    Serial.println("Scanning for I2C devices ...");
    Serial.print("      ");
    for (int i = 0; i < 0x10; i++)
    {
        Serial.printf("0x%02X|", i);
    }
    uint8_t error;
    for (int j = 0; j < 0x80; j += 0x10)
    {
        Serial.println();
        Serial.printf("0x%02X |", j);
        for (int i = 0; i < 0x10; i++)
        {
            Wire.beginTransmission(i | j);
            error = Wire.endTransmission();
            if (error == 0)
                Serial.printf("0x%02X|", i | j);
            else
                Serial.print(" -- |");
        }
    }
    Serial.println();
}

void wifi_test(void)
{
    String text;
    int wifi_num = 0;

    gfx->setTextSize(2);
    gfx->setTextColor(GREEN);

    Serial.println("\nScanning wifi");
    gfx->printf("\n\n   Scanning wifi\n");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    wifi_num = WiFi.scanNetworks();
    if (wifi_num == 0)
    {
        text = "\n   WiFi scan complete !\n   No wifi discovered.\n";
    }
    else
    {
        text = "\n   WiFi scan complete !\n";
        text += "   ";
        text += wifi_num;
        text += "    wifi discovered.\n\n";

        for (int i = 0; i < wifi_num; i++)
        {
            text += "   ";
            text += (i + 1);
            text += ": ";
            text += WiFi.SSID(i);
            text += " (";
            text += WiFi.RSSI(i);
            text += ")";
            text += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " \n" : "*\n";
            delay(10);
        }
    }

    Serial.println(text);
    gfx->println(text);

    text = "Connecting to ";
    Serial.print("Connecting to ");
    gfx->printf("   Connecting to\n");
    text += "   ";
    text += WIFI_SSID;
    text += "\n";

    Serial.print(WIFI_SSID);
    gfx->printf("   %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t last_tick = millis();
    uint32_t i = 0;

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        gfx->printf(".");
        text += ".";
        delay(100);

        if (millis() - last_tick > WIFI_CONNECT_WAIT_MAX)
        {
            wifi_connection_failure_flag = true;
            break;
        }
    }

    if (!wifi_connection_failure_flag)
    {
        text += "\n   The connection was successful ! \nTakes ";
        Serial.print("\n   The connection was successful ! \nTakes ");
        gfx->printf("\n   The connection was successful ! \n   Takes ");
        text += millis() - last_tick;
        Serial.print(millis() - last_tick);
        gfx->print(millis() - last_tick);
        text += " ms\n";
        Serial.println(" ms\n");
        gfx->printf(" ms\n");

        gfx->setTextSize(3);
        gfx->setTextColor(GREEN);
        gfx->printf("\n\n\n   Wifi test passed!");
    }
    else
    {
        gfx->setTextSize(3);
        gfx->setTextColor(RED);
        gfx->printf("\n\n\n   Wifi test error!\n");
    }
}

void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time!");
        gfx->fillRoundRect(10, 10, 200, 80, 15, WHITE);
        gfx->setCursor(20, 40);
        gfx->setTextSize(1);
        gfx->setTextColor(RED);
        gfx->print("Failed to obtain time!");
        return;
    }
    Serial.println("Get time success");
    Serial.println(&timeinfo, "%A,%B %d %Y %H:%M:%S"); // Format Output
    gfx->fillRoundRect(10, 10, 200, 80, 15, WHITE);
    gfx->setCursor(20, 20);
    gfx->setTextSize(2);
    gfx->setTextColor(PINK);
    gfx->print(&timeinfo, " %Y");
    gfx->setCursor(20, 40);
    gfx->print(&timeinfo, "%B %d");
    gfx->setCursor(20, 60);
    gfx->print(&timeinfo, "%H:%M:%S");
}

void SD_Test(void)
{
    uint8_t SelfLocking_Flag = 0;
    uint8_t cardType = 0;
    uint64_t cardSize = 0;
    uint8_t numSectors = 0;

    while (!SD.begin(SD_CS, SPI, 40000000) && SelfLocking_Flag != 3) // SD boots
    {
        SelfLocking_Flag++;
        gfx->setTextSize(2);
        gfx->setTextColor(GREEN);
        Serial.println("   Detecting SD card");
        gfx->printf("\n\n   Detecting SD card\n");

        gfx->setTextColor(RED);
        Serial.println("   SD card initialization failed !");
        gfx->printf("   SD card initialization failed !\n");
        delay(100);

        Serial.println(".");
        gfx->printf("   .");
        delay(100);

        Serial.println(".");
        gfx->printf(".");
        delay(100);

        Serial.println(".");
        gfx->printf(".");
        delay(100);

        Serial.println(".");
        gfx->printf(".");
        delay(100);

        Serial.println(".");
        gfx->printf(".");
        delay(100);

        Serial.println(".");
        gfx->printf(".");
        delay(100);

        gfx->printf("\n");
    }
    if (SelfLocking_Flag < 3)
    {
        delay(50); // Wait for the SD card
        gfx->setTextSize(2);
        gfx->setTextColor(GREEN);

        Serial.println("   SD card initialization successful !");
        gfx->printf("\n\n   SD card initialization successful !\n");
        delay(100);

        cardType = SD.cardType();
        cardSize = SD.cardSize() / (1024 * 1024);
        numSectors = SD.numSectors();
        switch (cardType)
        {
        case CARD_NONE:
            Serial.println("   No SD card attached");
            gfx->printf("   No SD card attached\n");

            gfx->setTextSize(3);
            gfx->setTextColor(RED);
            gfx->printf("\n\n\n   SD card test error!\n");
            delay(100);

            break;
        case CARD_MMC:
            Serial.print("   SD Card Type: ");
            Serial.println("   MMC");
            Serial.printf("   SD Card Size: %lluMB\n", cardSize);
            gfx->printf("   SD Card Type: MMC\n   SD Card Size: %lluMB\n", cardSize);

            gfx->setTextSize(3);
            gfx->setTextColor(GREEN);
            gfx->printf("\n\n\n   SD card test passed!\n");
            delay(100);

            break;
        case CARD_SD:
            Serial.print("   SD Card Type: ");
            Serial.println("   SDSC");
            Serial.printf("   SD Card Size: %lluMB\n", cardSize);
            gfx->printf("   SD Card Type: SDSC\n   SD Card Size: %lluMB\n", cardSize);

            gfx->setTextSize(3);
            gfx->setTextColor(GREEN);
            gfx->printf("\n\n\n   SD card tested correct!\n");
            delay(100);

            break;
        case CARD_SDHC:
            Serial.print("   SD Card Type: ");
            Serial.println("SDHC");
            Serial.printf("   SD Card Size: %lluMB\n", cardSize);
            gfx->printf("   SD Card Type: SDHC\n   SD Card Size: %lluMB\n", cardSize);

            gfx->setTextSize(3);
            gfx->setTextColor(GREEN);
            gfx->printf("\n\n\n   SD card tested correct!\n");
            delay(100);

            break;
        default:
            Serial.println("   UNKNOWN");
            gfx->printf("   UNKNOWN");

            gfx->setTextSize(3);
            gfx->setTextColor(RED);
            gfx->printf("\n\n\n   SD card test error!\n");
            delay(100);

            break;
        }
    }
    else
    {
        gfx->setTextSize(3);
        gfx->setTextColor(RED);
        gfx->printf("\n\n\n   SD card test error!\n");
    }
    SD.end();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(KEY1, INPUT_PULLUP);
    pinMode(KEY2, INPUT_PULLUP);

    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, LOW);

    pinMode(TOUCH_INT, INPUT_PULLUP);
    digitalWrite(TOUCH_INT, HIGH);

    attachInterrupt(
        TOUCH_INT,
        []
        {
            Get_int = 1;
            Serial.println("get_int");
        },
        FALLING); // Triggered every 1ms

    ledcAttachPin(LCD_BL, 1);
    ledcSetup(1, 1000, 8);
    ledcWrite(1, 0);

    Wire.begin(IIC_SDA, IIC_SCL);
    // scan_i2c_device(Wire);

    gfx->begin();
    gfx->fillScreen(WHITE);

    gfx->setCursor(180, 200);
    gfx->setTextSize(4);
    gfx->setTextColor(PINK);
    gfx->printf("Ciallo");

    for (int j = 0; j < 2; j++)
    {
        delay(2000);
        for (int i = 0; i <= 255; i++)
        {
            ledcWrite(1, i);
            delay(2);
        }
        for (int i = 255; i >= 0; i--)
        {
            ledcWrite(1, i);
            delay(5);
        }
    }
    delay(2000);
    for (int i = 0; i <= 255; i++)
    {
        ledcWrite(1, i);
        delay(2);
    }
    delay(3000);

    // Wifi Test
    gfx->fillScreen(BLACK);
    gfx->setCursor(0, 0);
    wifi_test();

    if (!wifi_connection_failure_flag)
    {
        // Obtain and set the time from the network time server
        // After successful acquisition, the chip will use the RTC clock to update the holding time
        configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);
    }

    delay(5000);

    // SD Test
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS); // SPI boots
    gfx->fillScreen(BLACK);
    gfx->setCursor(0, 0);
    SD_Test();
    delay(5000);

    // Touch
    touch.init();
    touch.read();
    TP_Point t = touch.getPoint(0);
    Last_X = t.x;
    Last_Y = t.y;

    gfx->fillScreen(RED);
    // gfx->drawRect(10, 10, 460, 460, RED);
    // gfx->drawRect(0, 0, 480, 480, RED);
    gfx->fillRect(10, 10, 460, 460, GREEN);

    gfx->setCursor(100, 200);
    gfx->setTextSize(3);
    gfx->setTextColor(PURPLE);
    gfx->printf("Edge Detection");

    gfx->setTextSize(2);
    gfx->setTextColor(NAVY);

    delay(1000);
}

void loop()
{
    if (Display_Start == 1)
    {
        if (millis() > wifi_time_delay)
        {
            if (!wifi_connection_failure_flag)
            {
                printLocalTime();
            }
            else
            {
                gfx->fillRoundRect(10, 10, 200, 80, 15, WHITE);
                gfx->setCursor(20, 40);
                gfx->setTextSize(1);
                gfx->setTextColor(RED);
                gfx->print("Not connected to the network");
            }

            wifi_time_delay = millis() + 500;
        }
    }

    if (Get_int)
    {
        Get_int = 0;
        touch.read();
        Touch_Num = touch.getPointNum();
        if (Touch_Num > 0)
        {
            Touch_Flag = 1;
            Touch_Num = 0;
        }
    }
    if (Touch_Flag == 1)
    {
        gfx->setTextSize(2);
        gfx->setTextColor(NAVY);
        Touch_Flag = 0;
        TP_Point t = touch.getPoint(0);
        if (Last_X != t.x && Last_Y != t.y)
        {
            gfx->setCursor(t.x, t.y);

            switch (Image_Flag)
            {
            case 0:
                gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_1, 480, 480); // RGB
                Serial.printf("[1] point x: %d  point y: %d \r\n", t.x, t.y);
                gfx->printf("[1] point x: %d  point y: %d \r\n", t.x, t.y);
                break;
            case 1:
                gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_2, 480, 480); // RGB
                Serial.printf("[1] point x: %d  point y: %d \r\n", t.x, t.y);
                gfx->printf("[1] point x: %d  point y: %d \r\n", t.x, t.y);
                break;
            case 2:
                gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_3, 480, 480); // RGB
                Serial.printf("[1] point x: %d  point y: %d \r\n", t.x, t.y);
                gfx->printf("[1] point x: %d  point y: %d \r\n", t.x, t.y);
                break;
            case 3:
                gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_4, 480, 480); // RGB
                Serial.printf("[1] point x: %d  point y: %d \r\n", t.x, t.y);
                gfx->printf("[1] point x: %d  point y: %d \r\n", t.x, t.y);
                break;
            case 4:
                gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_5, 480, 480); // RGB
                Serial.printf("[1] point x: %d  point y: %d \r\n", t.x, t.y);
                gfx->printf("[1] point x: %d  point y: %d \r\n", t.x, t.y);
                break;

            default:
                break;
            }
            Image_Flag++;
            if (Image_Flag > 4)
            {
                Image_Flag = 0;
            }

            Last_X = t.x;
            Last_Y = t.y;

            Display_Start = 1;
        }
    }

    if (digitalRead(KEY1) == 0)
    {
        Display_Start = 1;
        delay(100);
        switch (Image_Flag)
        {
        case 0:
            gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_1, 480, 480); // RGB
            break;
        case 1:
            gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_2, 480, 480); // RGB
            break;
        case 2:
            gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_3, 480, 480); // RGB
            break;
        case 3:
            gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_4, 480, 480); // RGB
            break;
        case 4:
            gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_5, 480, 480); // RGB
            break;

        default:
            break;
        }
        Image_Flag++;
        if (Image_Flag > 4)
        {
            Image_Flag = 0;
        }
    }

    if (digitalRead(KEY2) == 0)
    {
        Display_Start = 1;
        delay(100);
        switch (Image_Flag)
        {
        case 0:
            gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_1, 480, 480); // RGB
            break;
        case 1:
            gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_2, 480, 480); // RGB
            break;
        case 2:
            gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_3, 480, 480); // RGB
            break;
        case 3:
            gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_4, 480, 480); // RGB
            break;
        case 4:
            gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_5, 480, 480); // RGB
            break;

        default:
            break;
        }
        Image_Flag--;
        if (Image_Flag < 0)
        {
            Image_Flag = 4;
        }
    }
}
