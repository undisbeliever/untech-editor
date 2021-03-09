/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "idstring.h"
#include "iterators.h"
#include "optional.h"
#include "vector-helpers.h"
#include <cassert>
#include <climits>
#include <memory>
#include <string>
#include <vector>

namespace UnTech {

template <typename T>
class NamedList {
public:
    using container = typename std::vector<T>;

    using value_type = T;
    using iterator = typename container::iterator;
    using const_iterator = typename container::const_iterator;
    using size_type = typename container::size_type;

private:
    container _list;

public:
    NamedList() = default;
    ~NamedList() = default;

    NamedList(const NamedList&) = default;
    NamedList(NamedList&&) = default;
    NamedList& operator=(const NamedList& other) = default;
    NamedList& operator=(NamedList&& other) = default;

    bool empty() const { return _list.empty(); }
    size_type size() const { return _list.size(); }

    // returns INT_MAX if name is not found
    size_type indexOf(const std::string& name) const
    {
        for (const auto [i, item] : const_enumerate(_list)) {
            if (item.name == name) {
                return i;
            }
        }
        return INT_MAX;
    }

    // NOTE: pointer may be null
    // pointer is valid until item is removed or replaced
    optional<T&> find(const std::string& name)
    {
        for (T& i : _list) {
            if (i.name == name) {
                return i;
            }
        }
        return {};
    }

    // NOTE: pointer may be null
    // pointer is valid until item is removed or replaced
    optional<const T&> find(const std::string& name) const
    {
        for (const T& i : _list) {
            if (i.name == name) {
                return i;
            }
        }
        return {};
    }

    // pointer is valid until item is removed or replaced
    T& at(size_type index) { return _list.at(index); }
    const T& at(size_type index) const { return _list.at(index); }

    iterator begin() { return _list.begin(); }
    iterator end() { return _list.end(); }
    const_iterator begin() const { return _list.begin(); }
    const_iterator end() const { return _list.end(); }
    const_iterator cbegin() const { return _list.cbegin(); }
    const_iterator cend() const { return _list.cend(); }

    T& front() { return _list.front(); }
    T& back() { return _list.back(); }
    const T& front() const { return _list.front(); }
    const T& back() const { return _list.back(); }

    void resize(size_type n) { _list.resize(n); }
    void reserve(size_type capacity) { _list.reserve(capacity); }

    void emplace_back()
    {
        _list.emplace_back();
    }

    void insert_back()
    {
        _list.emplace_back();
    }

    void insert_back(const T& v)
    {
        _list.push_back(v);
    }

    void insert_back(T&& v)
    {
        _list.push_back(std::move(v));
    }

    void insert(size_type index, const T& v)
    {
        assert(index <= _list.size());
        _list.emplace(_list.begin() + index, v);
    }

    void remove(size_type index)
    {
        assert(index < _list.size());
        _list.erase(_list.begin() + index);
    }

    void moveItem(size_type from, size_type to)
    {
        moveVectorItem(from, to, _list);
    }

    void insert(const_iterator it, const T& value) { _list.insert(it, value); }
    void erase(const_iterator it) { _list.erase(it); }

    bool operator==(const NamedList& o) const
    {
        return _list == o._list;
    }
    bool operator!=(const NamedList& o) const { return !(*this == o); }
};
}
