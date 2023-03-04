/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <algorithm>
#include <type_traits>
#include <vector>

namespace UnTech {

template <typename T, class Compare = std::less<T>>
class vectorset {
    static_assert(not std::is_pointer_v<T>);

    using container = typename std::vector<T>;

public:
    using value_type = T;
    using size_type = typename container::size_type;
    using const_iterator = typename container::const_iterator;
    using const_reverse_iterator = typename container::const_reverse_iterator;

private:
    container _vector;
    Compare _comp;

public:
    explicit vectorset() = default;
    ~vectorset() = default;
    vectorset(const vectorset&) = default;
    vectorset(vectorset&&) noexcept = default;
    vectorset& operator=(const vectorset&) = default;
    vectorset& operator=(vectorset&&) noexcept = default;

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

    const std::vector<T>& vector() const { return _vector; }

    const_iterator find(const T& value)
    {
        const_iterator it = std::lower_bound(_vector.cbegin(), _vector.cend(), value, _comp);
        if (it != _vector.end() && *it == value) {
            return it;
        }
        else {
            return _vector.end();
        }
    }

    bool contains(const T& value) const
    {
        return std::binary_search(_vector.cbegin(), _vector.cend(), value, _comp);
    }

    bool insert(const T& value)
    {
        auto it = std::lower_bound(_vector.begin(), _vector.end(), value, _comp);
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
        auto it = std::lower_bound(_vector.begin(), _vector.end(), value, _comp);
        if (it != _vector.end() && *it == value) {
            _vector.erase(it);
            return true;
        }

        return false;
    }

    [[nodiscard]] bool empty() const { return _vector.empty(); }
    [[nodiscard]] size_t size() const { return _vector.size(); }
    size_type capacity() const { return _vector.capacity(); }

    void clear() { _vector.clear(); }
    void reserve(size_type cap) { _vector.reserve(cap); }

    const T& front() const { return _vector.front(); }
    const T& back() const { return _vector.back(); }

    const_iterator begin() const { return _vector.cbegin(); }
    const_iterator end() const { return _vector.cend(); }
    const_iterator cbegin() const { return _vector.cbegin(); }
    const_iterator cend() const { return _vector.cend(); }

    const_reverse_iterator rbegin() const { return _vector.crbegin(); }
    const_reverse_iterator rend() const { return _vector.crend(); }

    // Cannot use default here, there is no viable comparison function for `std::less<T>`.
    bool operator==(const vectorset<T, Compare>& o) const { return _vector == o._vector; }

private:
    void sortAndRemoveDuplicates()
    {
        if (std::is_sorted(_vector.begin(), _vector.end(), _comp) == false) {
            std::sort(_vector.begin(), _vector.end(), _comp);
        }

        _vector.erase(std::unique(_vector.begin(), _vector.end()), _vector.end());
    }
};

}
