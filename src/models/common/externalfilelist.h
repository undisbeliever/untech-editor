/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "idstring.h"
#include "iterators.h"
#include "optional_ref.h"
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

    // Cannot use default here.  I want std::unique_ptr value comparison.
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
};

template <typename T>
class ExternalFileList {
public:
    using Item = ExternalFileItem<T>;
    using container = typename std::vector<ExternalFileItem<T>>;
    using iterator = typename container::iterator;
    using const_iterator = typename container::const_iterator;
    using value_type = T;
    using size_type = typename container::size_type;

private:
    container _list;

public:
    ExternalFileList() = default;
    ~ExternalFileList() = default;

    // Allow moving
    ExternalFileList(ExternalFileList&&) noexcept = default;
    ExternalFileList& operator=(ExternalFileList&& other) noexcept = default;

    ExternalFileList(const ExternalFileList&) = delete;
    ExternalFileList& operator=(const ExternalFileList& other) = delete;

    size_type size() const { return _list.size(); }

    // return reference is valid until the item is removed or reloaded
    optional_ref<T&> find(const idstring& name)
    {
        for (const Item& item : _list) {
            if (item.value && item.value->name == name) {
                return item.value;
            }
        }
        return std::nullopt;
    }

    // return reference is valid until the item is removed or reloaded
    optional_ref<const T&> find(const idstring& name) const
    {
        for (const Item& item : _list) {
            if (item.value && item.value->name == name) {
                return item.value;
            }
        }
        return std::nullopt;
    }

    std::optional<size_type> indexOf(const idstring& name) const
    {
        for (const auto [i, item] : const_enumerate(_list)) {
            if (item.value && item.value->name == name) {
                return i;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] bool contains(const idstring& name) const
    {
        return std::any_of(_list.begin(), _list.end(),
                           [&](const auto& i) { return i.value && i.value->name == name; });
    }

    Item& item(size_type index) { return _list.at(index); }
    const Item& item(size_type index) const { return _list.at(index); }

    // return reference is valid until the item is removed or reloaded
    optional_ref<T&> at(size_type index) { return _list.at(index).value; }
    optional_ref<const T&> at(size_type index) const { return _list.at(index).value; }

    iterator begin() { return _list.begin(); }
    iterator end() { return _list.end(); }
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

    bool operator==(const ExternalFileList&) const = default;
};
}
