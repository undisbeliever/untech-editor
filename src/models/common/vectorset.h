/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <algorithm>
#include <functional>
#include <vector>

namespace UnTech {

template <typename T>
class vectorset {
    using container = typename std::vector<T>;

    using size_type = typename container::size_type;
    using const_iterator = typename container::const_iterator;
    using const_reverse_iterator = typename container::const_reverse_iterator;

private:
    container _vector;

public:
    explicit vectorset() = default;
    ~vectorset() = default;
    vectorset(const vectorset&) = default;
    vectorset(vectorset&&) = default;
    vectorset& operator=(const vectorset&) = default;
    vectorset& operator=(vectorset&&) = default;

    vectorset(const std::vector<T>& v)
        : _vector(v)
    {
        sortAndRemoveDuplicates();
    }

    vectorset(const std::vector<T>&& v)
        : _vector(std::move(v))
    {
        sortAndRemoveDuplicates();
    }

    vectorset(std::initializer_list<T> init)
        : _vector(init)
    {
        sortAndRemoveDuplicates();
    }

    const_iterator find(const T& value)
    {
        const_iterator it = std::lower_bound(_vector.cbegin(), _vector.cend(), value);
        if (it != _vector.end() && *it == value) {
            return it;
        }
        else {
            return _vector.end();
        }
    }

    bool contains(const T& value) const
    {
        return std::binary_search(_vector.cbegin(), _vector.cend(), value);
    }

    bool insert(const T& value)
    {
        auto it = std::lower_bound(_vector.begin(), _vector.end(), value);
        if (it == _vector.end() || *it != value) {
            _vector.insert(it, value);
            return true;
        }
        else {
            return false;
        }
    }

    const_iterator erase(const_iterator it)
    {
        return _vector.erase(it);
    }

    bool erase(const T& value)
    {
        auto it = std::lower_bound(_vector.begin(), _vector.end(), value);
        if (it != _vector.end() && *it == value) {
            _vector.erase(it);
            return true;
        }

        return false;
    }

    bool empty() const { return _vector.empty(); }
    size_t size() const { return _vector.size(); }
    size_type capacity() const { return _vector.capacity(); }

    void clear() { _vector.clear(); }
    void reserve(size_type cap) { _vector.reserve(cap); }

    const T& front() const { return _vector.front(); }
    const T& back() const { return _vector.back(); }

    const_iterator begin() const { return _vector.cbegin(); }
    const_iterator end() const { return _vector.cend(); }

    const_reverse_iterator rbegin() const { return _vector.crbegin(); }
    const_reverse_iterator rend() const { return _vector.crend(); }

    bool operator==(const vectorset<T>& o) const { return _vector == o._vector; }
    bool operator!=(const vectorset<T>& o) const { return _vector != o._vector; }
    bool operator<(const vectorset<T>& o) const { return _vector < o._vector; }
    bool operator<=(const vectorset<T>& o) const { return _vector <= o._vector; }
    bool operator>(const vectorset<T>& o) const { return _vector > o._vector; }
    bool operator>=(const vectorset<T>& o) const { return _vector >= o._vector; }

private:
    void sortAndRemoveDuplicates()
    {
        if (std::is_sorted(_vector.begin(), _vector.end()) == false) {
            std::sort(_vector.begin(), _vector.end());
        }

        _vector.erase(std::unique(_vector.begin(), _vector.end()), _vector.end());
    }
};

// vectorset<T*> cannot be iterated in a deterministic order.
//
// Thus I have disabled iteration on vectorset<T*> to prevent future-me from
// using vectorset in a non-deterministic manner.
template <typename T>
class vectorset<T*> {
    using container = typename std::vector<T*>;

    using size_type = typename container::size_type;

private:
    container _vector;

public:
    explicit vectorset() = default;
    ~vectorset() = default;
    vectorset(const vectorset&) = default;
    vectorset(vectorset&&) = default;
    vectorset& operator=(const vectorset&) = default;
    vectorset& operator=(vectorset&&) = default;

    vectorset(const std::vector<T*>& v)
        : _vector(v)
    {
        sortAndRemoveDuplicates();
    }

    vectorset(const std::vector<T*>&& v)
        : _vector(std::move(v))
    {
        sortAndRemoveDuplicates();
    }

    vectorset(std::initializer_list<T*> init)
        : _vector(init)
    {
        sortAndRemoveDuplicates();
    }

    bool contains(T* const value) const
    {
        return std::binary_search(_vector.cbegin(), _vector.cend(), value);
    }

    bool insert(T* value)
    {
        auto it = std::lower_bound(_vector.begin(), _vector.end(), value);
        if (it == _vector.end() || *it != value) {
            _vector.insert(it, value);
            return true;
        }
        else {
            return false;
        }
    }

    bool erase(T* value)
    {
        auto it = std::lower_bound(_vector.begin(), _vector.end(), value);
        if (it != _vector.end() && *it == value) {
            _vector.erase(it);
            return true;
        }

        return false;
    }

    bool empty() const { return _vector.empty(); }
    size_t size() const { return _vector.size(); }
    size_type capacity() const { return _vector.capacity(); }

    void clear() { _vector.clear(); }
    void reserve(size_type cap) { _vector.reserve(cap); }

    const T& front() const { return _vector.front(); }
    const T& back() const { return _vector.back(); }

    bool operator==(const vectorset<T*>& o) const { return _vector == o._vector; }
    bool operator!=(const vectorset<T*>& o) const { return _vector != o._vector; }
    bool operator<(const vectorset<T*>& o) const { return _vector < o._vector; }
    bool operator<=(const vectorset<T*>& o) const { return _vector <= o._vector; }
    bool operator>(const vectorset<T*>& o) const { return _vector > o._vector; }
    bool operator>=(const vectorset<T*>& o) const { return _vector >= o._vector; }

private:
    void sortAndRemoveDuplicates()
    {
        if (std::is_sorted(_vector.begin(), _vector.end()) == false) {
            std::sort(_vector.begin(), _vector.end());
        }

        _vector.erase(std::unique(_vector.begin(), _vector.end()), _vector.end());
    }
};
}
