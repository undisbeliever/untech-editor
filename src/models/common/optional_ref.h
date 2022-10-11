/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <optional>
#include <type_traits>

namespace UnTech {

// An optional reference class.
// Once created, it cannot be changed or moved.
// Intended for function return values.
template <typename T>
requires std::is_lvalue_reference_v<T>
class optional_ref {
    static_assert(std::is_lvalue_reference_v<T>);

    using value_type = std::remove_reference_t<T>;

private:
    value_type* const _ptr;

public:
    // An optional_ref cannot be changed or moved
    optional_ref(const optional_ref&) = delete;
    optional_ref& operator=(const optional_ref&) = delete;
    optional_ref& operator=(optional_ref&&) = delete;
    optional_ref(optional_ref&&) = delete;

    // cppcheck-suppress noExplicitConstructor
    optional_ref(std::nullopt_t)
        : _ptr(nullptr)
    {
    }

    // cppcheck-suppress noExplicitConstructor
    optional_ref(value_type& v)
        : _ptr(&v)
    {
    }

    // cppcheck-suppress noExplicitConstructor
    optional_ref(const std::unique_ptr<value_type>& v)
        : _ptr(v.get())
    {
    }

    // cppcheck-suppress noExplicitConstructor
    optional_ref(const std::unique_ptr<std::remove_const_t<value_type>>& v) requires std::is_const_v<value_type>
        : _ptr(v.get())
    {
    }

    ~optional_ref() = default;

    [[nodiscard]] bool exists() const { return _ptr != nullptr; }
    explicit operator bool() const { return _ptr != nullptr; }

    inline const value_type& value() const
    {
        if (_ptr) {
            return *_ptr;
        }
        else {
            throw std::bad_optional_access();
        }
    }

    inline value_type& value()
    {
        if (_ptr) {
            return *_ptr;
        }
        else {
            throw std::bad_optional_access();
        }
    }

    inline const value_type& value_or(const value_type& defaultValue) const
    {
        return _ptr ? *_ptr : defaultValue;
    }

    // May return none
    inline value_type* ptr() { return _ptr; }

    inline value_type* operator->() { return &value(); }
    inline const value_type* operator->() const { return &value(); }
    inline value_type& operator*() { return value(); }
    inline const value_type& operator*() const { return value(); }

    inline value_type& operator()() { return value(); }
    inline const value_type& operator()() const { return value(); }
    inline const value_type& operator()(const value_type& defaultValue) const { return _ptr ? *_ptr : defaultValue; }
};

}
