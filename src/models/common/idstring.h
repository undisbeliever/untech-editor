/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <ostream>
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

    // fails silently
    explicit idstring(const std::string_view s)
        : data()
    {
        if (isValid(s)) {
            data = s;
        }
    }

    // fails silently
    // Required to fix a "call of overloaded ‘idstring(<brace-enclosed initializer list>)’ is ambiguous" compile error
    explicit idstring(const char* s)
        : idstring(std::string_view(s))
    {
    }

    // fails silently
    explicit idstring(std::string&& s)
        : data(std::move(s))
    {
        if (!isValid(data)) {
            data.clear();
        }
    }

    inline bool isValid() const { return !data.empty(); }

    // clang-format off
    inline const std::string& str() const { return data; }
    inline const char* c_str() const { return data.c_str(); }
    inline operator const std::string&() const { return data; }
    // clang-format on

    void clear() { data.clear(); }

    inline bool operator==(const idstring& o) const { return data == o.data; }
    inline bool operator!=(const idstring& o) const { return data != o.data; }
    inline bool operator<(const idstring& o) const { return data < o.data; }

    inline bool operator==(const std::string& o) const { return data == o; }
    inline bool operator!=(const std::string& o) const { return data != o; }
};

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

inline std::ostream& operator<<(std::ostream& o, const idstring& i)
{
    return o << i.str();
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
