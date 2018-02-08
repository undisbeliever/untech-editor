/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <stdexcept>

namespace UnTech {

template <typename T>
class optional {
public:
    ~optional() = default;
    optional(const optional&) = default;
    optional(optional&&) = default;
    optional& operator=(const optional&) = default;
    optional& operator=(optional&&) = default;

    optional()
        : _value()
        , _exists(false)
    {
    }

    optional(const T& v)
        : _value(v)
        , _exists(true)
    {
    }

    bool exists() const { return _exists; }
    explicit operator bool() const { return _exists; }

    inline const T& value() const
    {
        if (_exists) {
            return _value;
        }
        else {
            throw std::runtime_error("Bad optional access");
        }
    }

    inline const T& value_or(const T& defaultValue) const
    {
        if (_exists) {
            return _value;
        }
        else {
            throw defaultValue;
        }
    }

    inline constexpr optional& operator=(const T& v)
    {
        _value = v;
        _exists = true;
        return *this;
    }

    inline constexpr optional& operator=(T&& v)
    {
        _value = v;
        _exists = true;
        return *this;
    }

private:
    T _value;
    bool _exists;
};
}
