/**
 * @file sht30_convert.h
 * @brief SHT30 raw-sample conversion — pure logic, no I2C/hardware dependency.
 *
 * Split out from the driver so it can be unit-tested on the host (see
 * test/sht30_convert_test.cpp). The driver (sht30.h/.cpp) does the I2C
 * transfer and then calls convert()/valid() from here.
 */
#pragma once
#include <cstdint>
#include <cmath>

namespace sht30
{
    /// Sane room-sensor acceptance window (datasheet operating range).
    constexpr float TEMP_MIN_C = -40.0f;
    constexpr float TEMP_MAX_C = 125.0f;
    constexpr float HUM_MIN    = 0.0f;
    constexpr float HUM_MAX    = 100.0f;

    /**
     * @brief Convert a 6-byte SHT30 single-shot sample to temperature/humidity.
     * @param d  {Tmsb, Tlsb, Tcrc, Hmsb, Hlsb, Hcrc}. CRC bytes are ignored.
     * @param tempC    [out] temperature in degrees Celsius.
     * @param humidity [out] relative humidity in percent.
     */
    inline void convert(const uint8_t d[6], float& tempC, float& humidity)
    {
        const uint16_t rawT = ((uint16_t)d[0] << 8) | d[1];
        const uint16_t rawH = ((uint16_t)d[3] << 8) | d[4];
        tempC    = -45.0f + 175.0f * ((float)rawT / 65535.0f);
        humidity = 100.0f * ((float)rawH / 65535.0f);
    }

    /// @return true when both readings fall inside the sane operating window.
    inline bool valid(float tempC, float humidity)
    {
        if (std::isnan(tempC) || std::isnan(humidity)) return false;
        return tempC >= TEMP_MIN_C && tempC <= TEMP_MAX_C &&
               humidity >= HUM_MIN && humidity <= HUM_MAX;
    }
}
