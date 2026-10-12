/**
 * @file sht30.h
 * @brief SHT30 temperature/humidity sensor driver (I2C 0x44).
 *
 * Single-shot, high-repeatability, clock-stretching disabled (cmd 0x2400).
 * Pure sample math lives in sht30_convert.h so it can be unit-tested; this
 * header owns only the I2C transfer. Lifted from the factory BBT routine in
 * devices.cpp so both share one code path.
 */
#pragma once
#include "sht30_convert.h"
#include "../i2c/I2C_Class.hpp"

namespace sht30
{
    /// @return true if a device ACKs at the SHT30 address on the given bus.
    bool present(I2C_Class* i2c = &In_I2C);

    /**
     * @brief Trigger a single-shot measurement and read it back.
     * @param tempC    [out] temperature in Celsius (untouched on failure).
     * @param humidity [out] relative humidity in percent (untouched on failure).
     * @param i2c      I2C bus (defaults to the internal bus).
     * @return true only when the I2C read succeeds AND the sample is valid().
     */
    bool read(float& tempC, float& humidity, I2C_Class* i2c = &In_I2C);
}
