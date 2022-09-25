/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <string>
#include <string_view>

namespace UnTech {
class idstring;
}
namespace ImGui {
bool InputIdstring(const char*, UnTech::idstring*);
}

namespace UnTech {

// Will ALWAYS contain valid data.
// Data structure fails silently
class idstring {
    // Allow ImGui::InputIdstring to access idstring internals.
    friend bool ImGui::InputIdstring(const char*, idstring*);

private:
    std::u8string data;

public:
    static constexpr bool isCharValid(const char c)
    {
        return ((c == '_'
                 || (c >= '0' && c <= '9')
                 || (c >= 'A' && c <= 'Z')
                 || (c >= 'a' && c <= 'z')));
    }

    static constexpr bool isCharInvalid(const char c) { return !isCharValid(c); }

    static constexpr bool isValid(std::u8string_view name)
    {
        if (name.empty()) {
            return false;
        }

        return std::all_of(name.begin(), name.end(), isCharValid);
    }

    static constexpr idstring fixup(const std::u8string& s)
    {
        idstring ret;

        ret.data = s;
        std::replace_if(ret.data.begin(), ret.data.end(), isCharInvalid, u8'_');

        return ret;
    }

public:
    ~idstring() = default;
    idstring(const idstring&) = default;
    idstring(idstring&&) = default;
    idstring& operator=(const idstring&) = default;
    idstring& operator=(idstring&&) = default;

    constexpr idstring() = default;

    static constexpr idstring fromString(std::u8string&& s)
    {
        idstring out;
        if (isValid(s)) {
            out.data = std::move(s);
        }

        return out;
    }

    static constexpr idstring fromString(const std::u8string_view s)
    {
        idstring out;
        if (isValid(s)) {
            out.data = s;
        }

        return out;
    }

    static constexpr idstring fromString(const char8_t* s)
    {
        return fromString(std::u8string_view(s));
    }

    inline bool isValid() const { return !data.empty(); }

    // clang-format off
    inline const std::u8string& str() const { return data; }
    inline const char8_t* c_str() const { return data.c_str(); }
    // clang-format on

    void clear() { data.clear(); }

    bool operator==(const idstring& o) const = default;
    auto operator<=>(const idstring& o) const = default;
};

inline idstring operator"" _id(const char8_t* str, const size_t size)
{
    const auto id = idstring::fromString(std::u8string_view(str, size));
    assert(id.isValid());
    return id;
}

// useful in generating user messages
inline std::u8string operator+(const char8_t* c, const idstring& i)
{
    return c + i.str();
}
inline std::u8string operator+(const idstring& i, const char8_t* c)
{
    return i.str() + c;
}
inline std::u8string operator+(const std::u8string& s, const idstring& i)
{
    return s + i.str();
}
inline std::u8string operator+(const idstring& i, const std::u8string& s)
{
    return i.str() + s;
}

}

namespace std {
template <>
struct hash<UnTech::idstring> {
    hash() = default;
    inline size_t operator()(const UnTech::idstring& id) const { return strHash(id.str()); };

private:
    hash<std::u8string> strHash;
};
}
