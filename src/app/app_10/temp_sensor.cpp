/**
 * @file temp_sensor.cpp
 * @brief App10 — Temperature / Humidity (SHT30)
 */
#include "temp_sensor.h"
#include "../../bsp/sensor/sht30.h"
#include <Arduino.h>

namespace MOONCAKE::APPS
{
    namespace {
        constexpr uint32_t READ_INTERVAL_MS = 1000;
        constexpr int      LABEL_X = 20;
        constexpr int      TEMP_Y  = 70;
        constexpr int      HUM_Y   = 150;
        constexpr int      VAL_X   = 20;   // value baseline, below each label
    }

    App10::App10(DEVICES* device)
        : _device(device)
    {
        setAppInfo().name = "Temp/Humidity";
    }

    void App10::onOpen()
    {
        _nextRead  = 0;              // sample immediately
        _lastTemp  = -1000.0f;
        _lastHum   = -1000.0f;
        _lastOk    = true;
        _everOk    = false;
        drawStatic();
    }

    void App10::drawStatic()
    {
        auto& lcd = _device->Lcd;
        lcd.fillScreen(TFT_BLACK);

        lcd.setFont(&fonts::efontCN_16);
        lcd.setTextSize(1);
        lcd.setTextColor(TFT_CYAN, TFT_BLACK);
        lcd.setCursor(LABEL_X, 20);
        lcd.printf("SHT30  Temp / Humidity");

        lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
        lcd.setCursor(LABEL_X, TEMP_Y - 20);
        lcd.printf("Temperature");
        lcd.setCursor(LABEL_X, HUM_Y - 20);
        lcd.printf("Humidity");

        lcd.setCursor(LABEL_X, 220);
        lcd.printf("Hold B to exit");
    }

    void App10::drawReadings(float tempC, float humidity, bool ok)
    {
        auto& lcd = _device->Lcd;

        if (!ok) {
            // Clear both value rows, show a single not-found message.
            lcd.fillRect(0, TEMP_Y - 4, 320, 110, TFT_BLACK);
            lcd.setFont(&fonts::efontCN_16);
            lcd.setTextSize(1);
            lcd.setTextColor(TFT_RED, TFT_BLACK);
            lcd.setCursor(LABEL_X, TEMP_Y + 20);
            lcd.printf(_everOk ? "SHT30 read error" : "SHT30 not found");
            return;
        }

        char buf[16];
        lcd.setFont(&fonts::Font4);
        lcd.setTextSize(2);

        // Temperature (Celsius only).
        lcd.fillRect(VAL_X, TEMP_Y, 280, 56, TFT_BLACK);
        lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        lcd.setCursor(VAL_X, TEMP_Y);
        snprintf(buf, sizeof(buf), "%.1f C", tempC);
        lcd.printf("%s", buf);

        // Humidity.
        lcd.fillRect(VAL_X, HUM_Y, 280, 56, TFT_BLACK);
        lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        lcd.setCursor(VAL_X, HUM_Y);
        snprintf(buf, sizeof(buf), "%.1f %%", humidity);
        lcd.printf("%s", buf);
    }

    void App10::onRunning()
    {
        const uint32_t now = millis();
        if ((int32_t)(now - _nextRead) < 0) return;   // not time yet
        _nextRead = now + READ_INTERVAL_MS;

        float t = 0, h = 0;
        const bool ok = sht30::read(t, h);

        if (!ok) {
            if (_lastOk) { drawReadings(0, 0, false); _lastOk = false; }
            return;
        }

        _everOk = true;
        // Redraw only on a meaningful change, or after an error/first paint.
        const bool changed = !_lastOk ||
                             fabsf(t - _lastTemp) > 0.1f ||
                             fabsf(h - _lastHum)  > 0.1f;
        if (changed) {
            if (!_lastOk) drawStatic();   // repaint labels after an error screen
            drawReadings(t, h, true);
            _lastTemp = t;
            _lastHum  = h;
            _lastOk   = true;
        }
    }

    void App10::onClose()
    {
        if (_device && _device->Lcd.width() > 0 && _device->Lcd.height() > 0) {
            _device->Lcd.setTextSize(1);
            _device->Lcd.fillScreen(TFT_BLACK);
        }
    }
}
