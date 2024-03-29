/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/exceptions.h"
#include <algorithm>
#include <string>
#include <vector>

namespace UnTech {

/**
 * EnumMap is a constant in-order map of strings to-and-from enum values.
 */
template <typename T>
class EnumMap {
public:
    using value_type = typename std::pair<std::u8string, T>;
    using container = typename std::vector<value_type>;
    using iterator = typename container::const_iterator;
    using const_iterator = typename container::const_iterator;
    using size_type = typename container::size_type;

private:
    const std::vector<value_type> _map;

public:
    EnumMap(std::initializer_list<value_type> l)
        : _map(l.begin(), l.end())
    {
    }

    ~EnumMap() = default;
    EnumMap(const EnumMap&) = delete;
    EnumMap(EnumMap&&) = delete;
    EnumMap& operator=(const EnumMap&) = delete;
    EnumMap& operator=(EnumMap&&) = delete;

    iterator find(const std::u8string& string) const
    {
        return std::find_if(_map.cbegin(), _map.cend(),
                            [&](const auto& p) { return p.first == string; });
    }

    iterator find(const std::u8string_view string) const
    {
        return std::find_if(_map.cbegin(), _map.cend(),
                            [&](const auto& p) { return p.first == string; });
    }

    iterator find(T value) const
    {
        return std::find_if(_map.cbegin(), _map.cend(),
                            [&](const auto& p) { return p.second == value; });
    }

    // throws out_of_range if string is not found
    T valueOf(const std::u8string& string) const
    {
        auto it = find(string);
        if (it == end()) {
            throw out_of_range(u8"Cannot convert `", string, u8"` to Enum");
        }
        return it->second;
    }

    // returns an empty string if value is not found
    const std::u8string& nameOf(T value) const
    {
        static const std::u8string emptyString;

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
};
}
