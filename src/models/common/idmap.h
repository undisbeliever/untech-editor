/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "idstring.h"
#include <map>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace UnTech {

/**
 * A custom mapping of `idstring -> T` that allows for easy
 * renaming/cloning/extracting of elements.
 *
 * Notable things about this class:
 *   * Internally uses std::unique_ptr so that that renames are fast and
 *     do not invalidate any pointers.
 *   * Elements can be extracted from the map.
 *   * Maintains both a hash map and a b-tree map so that lookups are
 *     fast, AND iteration is preformed in id order.
 *   * Iteration also goes through a custom forward iteration that
 *     silently dereferences the pointers into references.
 *   * Simple methods to test if an idstring is already in the list
 *   * Throws an exception when:
 *      * Trying to create a new element with an existing id
 *      * Trying to access an id that does not exist in the map
 *      * An id is invalid
 */
template <class T>
class idmap {
    using HashMapT = std::unordered_map<idstring, std::unique_ptr<T>>;
    using OrderedMapT = std::map<idstring, T*>;

    // Used for lookups, unique_ptr storage
    HashMapT _hashMap;

    // Used for in order iteration
    OrderedMapT _orderedMap;

public:
    // Access
    template <typename IT>
    class forward_reference_iterator {
        using value_type = std::pair<const typename IT::value_type::first_type,
                                     typename std::remove_pointer<typename IT::value_type::second_type>::type&>;

        IT _parent;

    public:
        forward_reference_iterator(IT it)
            : _parent(it)
        {
        }

        value_type operator*() const
        {
            return value_type(_parent->first, *(_parent->second));
        };

        forward_reference_iterator& operator++()
        {
            ++_parent;
            return *this;
        }
        bool operator==(const forward_reference_iterator& o) const { return this->_parent == o._parent; }
        bool operator!=(const forward_reference_iterator& o) const { return this->_parent != o._parent; }
    };

    using iterator = forward_reference_iterator<typename OrderedMapT::iterator>;
    using const_iterator = forward_reference_iterator<typename OrderedMapT::const_iterator>;

public:
    idmap() = default;
    ~idmap() = default;
    idmap(idmap&&) = default;
    idmap& operator=(idmap&&) = default;

    idmap(const idmap& other)
        : _hashMap()
        , _orderedMap()
    {
        // using ordered map should be nicer to the system

        for (const auto& it : other._orderedMap) {
            auto newElem = std::make_unique<T>(*it.second);

            this->_orderedMap[it.first] = newElem.get();
            this->_hashMap[it.first] = std::move(newElem);
        }
    }

    idmap& operator=(const idmap& other)
    {
        this->_orderedMap.clear();
        this->_hashMap.clear();

        // using ordered map should be nicer to the system

        for (const auto& it : other._orderedMap) {
            auto newElem = std::make_unique<T>(*it.second);

            this->_orderedMap[it.first] = newElem.get();
            this->_hashMap[it.first] = std::move(newElem);
        }

        return *this;
    }

    T& create(const idstring& newId)
    {
        if (!newId.isValid() || this->contains(newId)) {
            throw std::invalid_argument("Invalid newId");
        }

        auto newElem = std::make_unique<T>();
        T* newElemPtr = newElem.get();

        _hashMap[newId] = std::move(newElem);
        _orderedMap[newId] = newElemPtr;

        return *newElemPtr;
    }

    T& clone(const idstring& oldId, const idstring& newId)
    {
        const auto it = _hashMap.find(oldId);

        if (it == _hashMap.end()) {
            throw std::out_of_range("Invalid oldId");
        }
        if (!newId.isValid() || this->contains(newId)) {
            throw std::invalid_argument("Invalid newId");
        }

        auto newElem = std::make_unique<T>(*it->second);
        T* newElemPtr = newElem.get();

        _hashMap[newId] = std::move(newElem);
        _orderedMap[newId] = newElemPtr;

        return *newElemPtr;
    }

    // Fails silently
    // Pointers to the element are now invalidated
    void remove(const idstring& id)
    {
        _orderedMap.erase(id);
        _hashMap.erase(id);
    }

    void rename(const idstring& oldId, const idstring& newId)
    {
        auto it = _hashMap.find(oldId);

        if (it == _hashMap.end()) {
            throw std::out_of_range("id not found");
        }
        if (!newId.isValid() || contains(newId)) {
            throw std::invalid_argument("newId is invalid");
        }

        std::unique_ptr<T> elem = std::move(it->second);

        _orderedMap.erase(oldId);
        _orderedMap[newId] = elem.get();

        _hashMap.erase(it);
        _hashMap[newId] = std::move(elem);
    }

    std::unique_ptr<T> extractFrom(const idstring& id)
    {
        auto it = _hashMap.find(id);

        if (it == _hashMap.end()) {
            throw std::out_of_range("id not found");
        }

        std::unique_ptr<T> ret = std::move(it->second);

        _hashMap.erase(it);
        _orderedMap.erase(id);

        return std::move(ret);
    }

    // idmap now takes ownership of the unique_pointer
    void insertInto(const idstring& id, std::unique_ptr<T> e)
    {
        if (!id.isValid() || this->contains(id)) {
            throw std::invalid_argument("Invalid id");
        }

        if (e == nullptr) {
            throw std::invalid_argument("e is null");
        }

        _orderedMap[id] = e.get();
        _hashMap[id] = std::move(e);
    }

    bool contains(const idstring& id) const
    {
        auto it = _hashMap.find(id);
        return it != _hashMap.end();
    }

    // Returns nullptr if the id is invalid or not in map
    T* getPtr(const idstring& id)
    {
        const auto it = _hashMap.find(id);

        if (it != _hashMap.end()) {
            return it->second.get();
        }
        else {
            return nullptr;
        }
    }
    const T* getPtr(const idstring& id) const
    {
        const auto it = _hashMap.find(id);

        if (it != _hashMap.end()) {
            return it->second.get();
        }
        else {
            return nullptr;
        }
    }

    // Expose the map
    T& at(const idstring& id) { return *_hashMap.at(id).get(); }
    const T& at(const idstring& id) const { return *_hashMap.at(id).get(); }

    inline size_t size() const { return _hashMap.size(); }

    inline iterator begin() noexcept { return _orderedMap.begin(); }
    inline iterator end() noexcept { return _orderedMap.end(); }
    inline const_iterator begin() const noexcept { return _orderedMap.begin(); }
    inline const_iterator end() const noexcept { return _orderedMap.end(); }
};
}
