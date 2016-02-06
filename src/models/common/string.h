#ifndef _UNTECH_MODELS_COMMON_STRING_H_
#define _UNTECH_MODELS_COMMON_STRING_H_

#include <string>
#include <cstdlib>

namespace UnTech {
namespace String {

/**
 * @return true if str is a NULL terminated utf8 well formed string.
 */
bool checkUtf8WellFormed(const std::string& str);

static inline std::string& ltrim(std::string& s)
{
    size_t f = s.find_first_not_of(" \t\n\r", 0, 4);
    s.erase(0, f);
    return s;
}

static inline std::string& rtrim(std::string& s)
{
    size_t f = s.find_last_not_of(" \t\n\r", s.npos, 4);
    s.erase(0, f);
    return s;
}

// trim from both ends
static inline std::string& trim(std::string& s)
{
    return ltrim(rtrim(s));
}

/* Convert a string to an integer.
 * String may be encased in spaces.
 * Raises `std::invalid_argument` if the entire string is not a number.
 */
static inline std::pair<int, bool> toInt(const std::string& s)
{
    const char* cstr = s.c_str();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    int ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1) {
        return { 0, false };
    }
    return { ret, true };
}

/* Convert a string to an integer.
 * Returns a default value if given invalid input.
 * String may be encased in spaces.
 */
static inline int toInt(const std::string& s, int def)
{
    const char* cstr = s.c_str();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    int ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1) {
        return def;
    }
    return ret;
}

static inline std::pair<long, bool> toLong(const std::string& s)
{
    const char* cstr = s.c_str();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    long ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1) {
        return { 0, false };
    }
    return { ret, true };
}

static inline long toLong(const std::string& s, long def)
{
    const char* cstr = s.c_str();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    long ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1) {
        return def;
    }
    return ret;
}
}
}
#endif
