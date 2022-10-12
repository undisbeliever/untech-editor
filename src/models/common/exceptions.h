/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "stringbuilder.h"
#include <exception>

namespace UnTech {

class base_error : public std::exception {
private:
    const std::u8string message;

public:
    template <typename... Args>
    base_error(const Args&... args)
        : std::exception()
        , message(stringBuilder(args...))
    {
    }

    [[nodiscard]] const std::u8string& msg() const noexcept { return message; }

    [[nodiscard]] const char* what() const noexcept final
    {
        return reinterpret_cast<const char*>(message.c_str());
    }
};

class runtime_error : public base_error {
public:
    template <typename... Args>
    runtime_error(const Args&... args)
        : base_error(args...)
    {
    }
};

class logic_error : public base_error {
public:
    template <typename... Args>
    logic_error(const Args&... args)
        : base_error(args...)
    {
    }
};

class invalid_argument : public logic_error {
public:
    template <typename... Args>
    invalid_argument(const Args&... args)
        : logic_error(args...)
    {
    }
};

class out_of_range : public base_error {
public:
    template <typename... Args>
    out_of_range(const Args&... args)
        : base_error(args...)
    {
    }
};

}
