#pragma once

#include <ostream>
#include <string>

namespace UnTech {

// Will ALWAYS contain valid data.
// Data structure fails silently
class idstring {
    std::string data;

public:
    static bool isNameCharValid(const char c)
    {
        return ((c == '_'
                 || (c >= '0' && c <= '9')
                 || (c >= 'A' && c <= 'Z')
                 || (c >= 'a' && c <= 'z')));
    }

    static bool isValid(const std::string& name)
    {
        if (name.empty()) {
            return false;
        }

        for (const char& c : name) {
            if (!isNameCharValid(c)) {
                return false;
            }
        }

        return true;
    }

public:
    ~idstring() = default;
    idstring(const idstring&) = default;
    idstring(idstring&&) = default;
    idstring& operator=(const idstring&) = default;
    idstring& operator=(idstring&&) = default;

    idstring() = default;
    idstring(const std::string& s)
    {
        if (isValid(s)) {
            data = s;
        }
    }

    inline bool isValid() const { return !data.empty(); }

    inline const std::string& str() const { return data; }
    inline operator const std::string&() const { return data; }

    inline idstring& operator=(const std::string& s)
    {
        if (isValid(s)) {
            data = s;
        }
        return *this;
    }

    inline bool operator==(const idstring& o) const { return data == o.data; }
    inline bool operator!=(const idstring& o) const { return data != o.data; }
    inline bool operator<(const idstring& o) const { return data < o.data; }
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
