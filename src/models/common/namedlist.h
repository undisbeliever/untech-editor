/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "idstring.h"
#include "vector-helpers.h"
#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace UnTech {

// The items are contained within a unique_ptr so each item points to the same
// memory address, even if the list is modified.
template <typename T>
class NamedList {
public:
    using container = typename std::vector<std::unique_ptr<T>>;
    using const_iterator = typename container::const_iterator;
    using size_type = typename container::size_type;

private:
    container _list;

public:
    NamedList() = default;
    ~NamedList() = default;

    NamedList(const NamedList&) = delete;
    NamedList(NamedList&&) = delete;
    NamedList& operator=(const NamedList& other) = delete;
    NamedList& operator=(NamedList&& other) = delete;

    bool empty() const { return _list.empty(); }
    size_type size() const { return _list.size(); }

    // NOTE: pointer may be null
    // pointer is valid until item is removed or replaced
    T* find(const idstring& name)
    {
        for (const auto& item : _list) {
            if (item && item->name == name) {
                return item.get();
            }
        }
        return nullptr;
    }

    // NOTE: pointer may be null
    // pointer is valid until item is removed or replaced
    const T* find(const idstring& name) const
    {
        for (const auto& item : _list) {
            if (item && item->name == name) {
                return item.get();
            }
        }
        return nullptr;
    }

    // pointer is valid until item is removed or replaced
    T* at(size_type index) { return _list.at(index).get(); }
    const T* at(size_type index) const { return _list.at(index).get(); }

    const_iterator begin() const { return _list.begin(); }
    const_iterator end() const { return _list.end(); }
    const_iterator cbegin() const { return _list.cbegin(); }
    const_iterator cend() const { return _list.cend(); }

    T* front() { return _list.front().get(); }
    T* back() { return _list.back().get(); }
    const T* front() const { return _list.front().get(); }
    const T* back() const { return _list.back().get(); }

    void insert_back()
    {
        _list.emplace_back(std::make_unique<T>());
    }

    void insert_back(typename std::unique_ptr<T> v)
    {
        assert(v != nullptr);
        _list.emplace_back(std::move(v));
    }

    void insert(size_type index, typename std::unique_ptr<T> v)
    {
        assert(v != nullptr);
        assert(index <= _list.size());
        _list.emplace(_list.begin() + index, std::move(v));
    }

    void remove(size_type index)
    {
        assert(index < _list.size());
        _list.erase(_list.begin() + index);
    }

    std::unique_ptr<T> takeFrom(size_type index)
    {
        assert(index < _list.size());
        std::unique_ptr<T> ret = std::move(_list.at(index));
        _list.erase(_list.begin() + index);

        return ret;
    }

    void moveItem(size_type from, size_type to)
    {
        moveVectorItem(from, to, _list);
    }

    bool operator==(const NamedList& o) const
    {
        return _list.size() == o._list.size()
               && std::equal(_list.cbegin(), _list.cend(), o._list.cbegin(),
                             [](const std::unique_ptr<T>& a, const std::unique_ptr<T>& b) { return *a == *b; });
    }
    bool operator!=(const NamedList& o) const { return !(*this == o); }
};
}
