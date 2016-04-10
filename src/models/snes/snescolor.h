#ifndef _UNTECH_MODELS_SNES_SNESCOLOR_H_
#define _UNTECH_MODELS_SNES_SNESCOLOR_H_

#include "models/common/rgba.h"
#include <cstdint>

namespace UnTech {
namespace Snes {

class SnesColor {
public:
    SnesColor()
        : _rgb(0, 0, 0, 0xFF)
        , _data(0)
    {
    }

    SnesColor(const SnesColor& c)
        : _rgb(c._rgb)
        , _data(c._data)
    {
    }

    SnesColor(const rgba& color)
        : _rgb(0, 0, 0, 0xFF)
        , _data(0)
    {
        setRgb(color);
    }

    SnesColor(const uint16_t value)
        : _rgb(0, 0, 0, 0xFF)
        , _data(0)
    {
        setData(value);
    }

    inline const rgba& rgb() const { return _rgb; }
    inline const uint16_t& data() const { return _data; }

    inline uint8_t blue() const { return (_data >> 10) & 31; }
    inline uint8_t green() const { return (_data >> 5) & 31; }
    inline uint8_t red() const { return _data & 31; }

    void setRgb(const rgba color);
    void setData(const uint16_t data);

    void setBlue(const uint8_t blue);
    void setGreen(const uint8_t green);
    void setRed(const uint8_t red);

    inline bool operator==(const SnesColor& o) const
    {
        return _data == o._data;
    }

    inline bool operator!=(const SnesColor& o) const
    {
        return _data != o._data;
    }

private:
    void updateRgb();

private:
    rgba _rgb;
    uint16_t _data;
};
}
}
#endif
