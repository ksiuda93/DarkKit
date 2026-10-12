/**
 * @file temp_sensor.h
 * @brief App10 — Temperature / Humidity (SHT30, I2C 0x44)
 *
 * Live room temperature and humidity from the on-board SHT30. Reads once per
 * second (non-blocking gate on millis()), redraws only changed fields.
 */
#pragma once
#include <mooncake.h>
#include "../../bsp/devices.h"

using namespace mooncake;

namespace MOONCAKE::APPS
{
    class App10 : public AppAbility {
    public:
        App10(DEVICES* device);
        void onOpen() override;
        void onRunning() override;
        void onClose() override;
    private:
        void drawStatic();
        void drawReadings(float tempC, float humidity, bool ok);

        DEVICES* _device = nullptr;
        uint32_t _nextRead = 0;        ///< next millis() at which to sample
        float    _lastTemp = -1000.0f; ///< last drawn values (force first paint)
        float    _lastHum  = -1000.0f;
        bool     _lastOk   = true;
        bool     _everOk   = false;    ///< seen a good reading at least once
    };
}
