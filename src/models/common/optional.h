/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <stdexcept>

namespace UnTech {

class bad_optional_access : public std::exception {
public:
    explicit bad_optional_access() = default;
    virtual const char* what() const noexcept final;
};

// Should only be used with simple types
template <typename T>
class optional {
public:
    optional()
        : _value()
        , _exists(false)
    {
    }

    optional(T v)
        : _value(std::move(v))
        , _exists(true)
    {
    }

    ~optional() = default;
    optional(const optional&) = default;
    optional(optional&&) = default;
    optional& operator=(const optional&) = default;
    optional& operator=(optional&&) = default;

    inline constexpr optional& operator=(T v)
    {
        _value = std::move(v);
        _exists = true;
        return *this;
    }

    bool exists() const { return _exists; }
    explicit operator bool() const { return _exists; }

    inline T& value()
    {
        if (_exists) {
            return _value;
        }
        else {
            throw bad_optional_access();
        }
    }

    inline const T& value() const
    {
        if (_exists) {
            return _value;
        }
        else {
            throw bad_optional_access();
        }
    }

    inline const T& value_or(const T& defaultValue) const
    {
        return _exists ? _value : defaultValue;
    }

    inline T* operator->() { return &value(); }
    inline const T* operator->() const { return &value(); }
    inline T& operator*() { return value(); }
    inline const T& operator*() const { return value(); }

    inline T& operator()() { return value(); }
    inline const T& operator()() const { return value(); }
    inline const T& operator()(const T& defaultValue) const { return value_or(defaultValue); }

private:
    T _value;
    bool _exists;
};

// Once created an optional reference cannot be changed, only moved.
template <typename T>
class optional<T&> {
public:
    optional()
        : _ptr(nullptr)
    {
    }

    optional(T& v)
        : _ptr(&v)
    {
    }

    optional(const std::unique_ptr<T>& v)
        : _ptr(v.get())
    {
    }

    ~optional() = default;
    optional(optional&&) = default;

    optional(const optional&) = delete;
    optional& operator=(const optional&) = delete;
    optional& operator=(optional&&) = delete;

    bool exists() const { return _ptr != nullptr; }
    explicit operator bool() const { return _ptr != nullptr; }

    inline const T& value() const
    {
        if (_ptr) {
            return *_ptr;
        }
        else {
            throw bad_optional_access();
        }
    }

    inline T& value()
    {
        if (_ptr) {
            return *_ptr;
        }
        else {
            throw bad_optional_access();
        }
    }

    inline const T& value_or(const T& defaultValue) const
    {
        return _ptr ? *_ptr : defaultValue;
    }

    inline T* operator->() { return &value(); }
    inline const T* operator->() const { return &value(); }
    inline T& operator*() { return value(); }
    inline const T& operator*() const { return value(); }

    inline T& operator()() { return value(); }
    inline const T& operator()() const { return value(); }
    inline const T& operator()(const T& defaultValue) const { return _ptr ? *_ptr : defaultValue; }

private:
    T* const _ptr;
};
}
