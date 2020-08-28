/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "idstring.h"
#include <cassert>
#include <climits>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace UnTech {

template <typename T>
struct ExternalFileItem {
    std::filesystem::path filename;
    typename std::unique_ptr<T> value;

    // may throw an exception
    void loadFile();

    bool operator==(const ExternalFileItem& o) const
    {
        if (filename != o.filename) {
            return false;
        }
        if (value == nullptr && o.value != nullptr) {
            return false;
        }
        if (value != nullptr && o.value == nullptr) {
            return false;
        }

        if (value == nullptr && o.value == nullptr) {
            return true;
        }
        return *value == *o.value;
    }
    bool operator!=(const ExternalFileItem& o) const { return *this != o; }
};

template <typename T>
class ExternalFileList {
public:
    using Item = ExternalFileItem<T>;
    using container = typename std::vector<ExternalFileItem<T>>;
    using const_iterator = typename container::const_iterator;
    using value_type = T;
    using size_type = typename container::size_type;

private:
    container _list;

public:
    ExternalFileList() = default;
    ~ExternalFileList() = default;

    ExternalFileList(const ExternalFileList&) = delete;
    ExternalFileList(ExternalFileList&&) = delete;
    ExternalFileList& operator=(const ExternalFileList& other) = delete;
    ExternalFileList& operator=(ExternalFileList&& other) = delete;

    size_type size() const { return _list.size(); }

    // NOTE: pointer may be null
    // pointer is valid until item removed or reloaded
    T* find(const idstring& name)
    {
        for (const Item& item : _list) {
            if (item.value && item.value->name == name) {
                return item.value.get();
            }
        }
        return nullptr;
    }

    // NOTE: pointer may be null
    // pointer is valid until item removed or reloaded
    const T* find(const idstring& name) const
    {
        for (const Item& item : _list) {
            if (item.value && item.value->name == name) {
                return item.value.get();
            }
        }
        return nullptr;
    }

    // returns INT_MAX if name is not found
    size_type indexOf(const idstring& name) const
    {
        for (size_type i = 0; i < _list.size(); i++) {
            const auto& item = _list.at(i);

            if (item.value && item.value->name == name) {
                return i;
            }
        }
        return INT_MAX;
    }

    Item& item(size_type index) { return _list.at(index); }
    const Item& item(size_type index) const { return _list.at(index); }

    // NOTE: pointer may be null
    // pointer is valid until item is removed, replaced or reloaded
    T* at(size_type index) { return _list.at(index).value.get(); }
    const T* at(size_type index) const { return _list.at(index).value.get(); }

    const_iterator begin() const { return _list.begin(); }
    const_iterator end() const { return _list.end(); }
    const_iterator cbegin() const { return _list.cbegin(); }
    const_iterator cend() const { return _list.cend(); }

    void insert_back(const std::filesystem::path& filename, typename std::unique_ptr<T> v = nullptr)
    {
        _list.emplace_back(Item{ filename, std::move(v) });
    }

    void insert(size_type index, const std::filesystem::path& filename,
                typename std::unique_ptr<T> v = nullptr)
    {
        assert(index < _list.size());
        _list.emplace(_list.begin() + index, { filename, std::move(v) });
    }

    void remove(size_type index)
    {
        assert(index < _list.size());
        _list.erase(_list.begin() + index);
    }

    // may raise an exception
    void loadAllFiles()
    {
        for (auto& item : _list) {
            item.loadFile();
        }
    }

    bool operator==(const ExternalFileList& o) const { return _list == o._list; }
    bool operator!=(const ExternalFileList& o) const { return _list != o._list; }
};
}
