#pragma once

#include <stdexcept>

namespace UnTech {

template <typename T>
class optional {
public:
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

    optional(const optional<T>&) = default;
    optional(optional<T>&&) = default;
    ~optional() = default;

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

private:
    T _value;
    bool _exists;
};
}
