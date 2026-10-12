#include "../src/bsp/sensor/sht30_convert.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

#define CHECK(expr) do { if (!(expr)) { std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expr); std::exit(1); } } while (0)

static bool near(float a, float b, float eps = 0.05f) { return std::fabs(a - b) < eps; }

int main() {
    using namespace sht30;
    float t = 0, h = 0;

    // Datasheet linear maps:
    //   T  = -45 + 175 * raw/65535
    //   RH = 100 * raw/65535
    // rawTemp 0x6666 (26214) -> 25.0 C ; rawHum 0x8000 (32768) -> 50.0 %
    // Frame layout: Tmsb, Tlsb, Tcrc, Hmsb, Hlsb, Hcrc
    uint8_t frame[6] = {0x66, 0x66, 0x00, 0x80, 0x00, 0x00};
    convert(frame, t, h);
    CHECK(near(t, 25.0f));
    CHECK(near(h, 50.0f));
    CHECK(valid(t, h));

    // Minimum: raw 0x0000 -> T = -45 C, RH = 0 %
    uint8_t lo[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    convert(lo, t, h);
    CHECK(near(t, -45.0f));
    CHECK(near(h, 0.0f));
    CHECK(!valid(t, h));                  // -45 C below sane room-range floor (-40)

    // Maximum: raw 0xFFFF -> T = 130 C, RH = 100 %
    uint8_t hi[6] = {0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00};
    convert(hi, t, h);
    CHECK(near(t, 130.0f, 0.1f));
    CHECK(near(h, 100.0f));
    CHECK(!valid(t, h));                  // 130 C above 125 ceiling

    // Realistic room reading stays valid.
    uint8_t room[6] = {0x61, 0x00, 0x00, 0x9A, 0x00, 0x00};
    convert(room, t, h);
    CHECK(valid(t, h));
    CHECK(t > 0.0f && t < 40.0f);
    CHECK(h > 0.0f && h < 100.0f);

    // valid() range guards directly.
    CHECK(valid(-40.0f, 0.0f));
    CHECK(valid(125.0f, 100.0f));
    CHECK(!valid(-40.1f, 50.0f));
    CHECK(!valid(125.1f, 50.0f));
    CHECK(!valid(25.0f, -0.1f));
    CHECK(!valid(25.0f, 100.1f));
    CHECK(!valid(std::nanf(""), 50.0f));  // NaN never valid

    std::puts("SHT30 convert: datasheet maps, range bounds and NaN guard passed");
    return 0;
}
