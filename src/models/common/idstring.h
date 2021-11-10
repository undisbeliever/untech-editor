/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

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
    std::string data;

public:
    static bool isCharValid(const char c)
    {
        return ((c == '_'
                 || (c >= '0' && c <= '9')
                 || (c >= 'A' && c <= 'Z')
                 || (c >= 'a' && c <= 'z')));
    }

    static bool isValid(std::string_view name)
    {
        if (name.empty()) {
            return false;
        }

        for (const char& c : name) {
            if (!isCharValid(c)) {
                return false;
            }
        }

        return true;
    }

    static idstring fixup(const std::string& s)
    {
        idstring ret;

        ret.data = s;
        for (auto& c : ret.data) {
            if (!isCharValid(c)) {
                c = '_';
            }
        }

        return ret;
    }

public:
    ~idstring() = default;
    idstring(const idstring&) = default;
    idstring(idstring&&) = default;
    idstring& operator=(const idstring&) = default;
    idstring& operator=(idstring&&) = default;

    idstring() = default;

    static idstring fromString(std::string&& s)
    {
        idstring out;
        if (isValid(s)) {
            out.data = std::move(s);
        }

        return out;
    }

    static idstring fromString(const std::string_view s)
    {
        idstring out;
        if (isValid(s)) {
            out.data = s;
        }

        return out;
    }

    static idstring fromString(const char* s)
    {
        return fromString(std::string_view(s));
    }

    inline bool isValid() const { return !data.empty(); }

    // clang-format off
    inline const std::string& str() const { return data; }
    inline const char* c_str() const { return data.c_str(); }
    // clang-format on

    void clear() { data.clear(); }

    bool operator==(const idstring& o) const = default;
    auto operator<=>(const idstring& o) const = default;
};

inline idstring operator"" _id(const char* str, const size_t size)
{
    const auto id = idstring::fromString(std::string_view(str, size));
    assert(id.isValid());
    return id;
}

// useful in generating user messages
inline std::string operator+(const char* c, const idstring& i)
{
    return c + i.str();
}
inline std::string operator+(const idstring& i, const char* c)
{
    return i.str() + c;
}
inline std::string operator+(const std::string& s, const idstring& i)
{
    return s + i.str();
}
inline std::string operator+(const idstring& i, const std::string& s)
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
    hash<std::string> strHash;
};
}
