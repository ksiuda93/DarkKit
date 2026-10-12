#include "sht30.h"
#include "../config.h"
#include <Arduino.h>
#include <driver/i2c.h>

namespace sht30
{
    bool present(I2C_Class* i2c)
    {
        return i2c && i2c->isEnabled() && i2c->scanID(HAL_I2C_ADDR_SHT30);
    }

    bool read(float& tempC, float& humidity, I2C_Class* i2c)
    {
        if (!present(i2c)) return false;

        // Single-shot, high repeatability, clock stretching disabled: cmd 0x2400.
        if (!i2c->writeRegister8(HAL_I2C_ADDR_SHT30, 0x24, 0x00)) return false;
        delay(20);  // measurement time ~15 ms

        // SHT30 returns 6 bytes to a plain read (no register pointer), so drive
        // the I2C command link directly on this bus's port.
        uint8_t data[6] = {};
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (HAL_I2C_ADDR_SHT30 << 1) | I2C_MASTER_READ, true);
        i2c_master_read(cmd, data, 5, I2C_MASTER_ACK);
        i2c_master_read_byte(cmd, &data[5], I2C_MASTER_NACK);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(i2c->getPort(), cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);
        if (err != ESP_OK) return false;

        float t, h;
        convert(data, t, h);
        if (!valid(t, h)) return false;

        tempC = t;
        humidity = h;
        return true;
    }
}
