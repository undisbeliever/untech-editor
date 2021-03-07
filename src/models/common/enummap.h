/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/stringbuilder.h"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace UnTech {

/**
 * EnumMap is a constant in-order map of strings to-and-from enum values.
 */
template <typename T>
class EnumMap {
public:
    using value_type = typename std::pair<std::string, T>;
    using container = typename std::vector<value_type>;
    using iterator = typename container::const_iterator;
    using const_iterator = typename container::const_iterator;
    using size_type = typename container::size_type;

    EnumMap(std::initializer_list<value_type> l)
        : _map(l.begin(), l.end())
    {
    }

    ~EnumMap() = default;
    EnumMap(const EnumMap&) = delete;
    EnumMap(EnumMap&&) = delete;
    EnumMap& operator=(const EnumMap&) = delete;
    EnumMap& operator=(EnumMap&&) = delete;

    iterator find(const std::string& string) const
    {
        return std::find_if(_map.cbegin(), _map.cend(),
                            [&](auto& p) { return p.first == string; });
    }

    iterator find(T value) const
    {
        return std::find_if(_map.cbegin(), _map.cend(),
                            [&](auto& p) { return p.second == value; });
    }

    // throws out_of_range if string is not found
    T valueOf(const std::string& string) const
    {
        auto it = find(string);
        if (it == end()) {
            throw std::out_of_range(stringBuilder("Cannot convert `", string, "` to Enum"));
        }
        return it->second;
    }

    // returns an empty string if value is not found
    const std::string& nameOf(T value) const
    {
        static const std::string emptyString;

        auto it = find(value);
        if (it == end()) {
            return emptyString;
        }
        return it->first;
    }

    size_type size() const { return _map.size(); }

    inline iterator begin() const { return _map.cbegin(); }
    inline iterator end() const { return _map.cend(); }
    inline const_iterator cbegin() const { return _map.cbegin(); }
    inline const_iterator cend() const { return _map.cend(); }

private:
    const std::vector<value_type> _map;
};
}
