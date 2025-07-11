/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cassert>
#include <iterator>
#include <limits>
#include <utility>

template <typename It, typename size_type>
class enumerate_iterator {
public:
    using value_type = std::pair<size_type, typename std::iterator_traits<It>::reference>;

private:
    size_type index;
    It iterator;

public:
    enumerate_iterator(size_type index, It iterator)
        : index(index)
        , iterator(iterator)
    {
    }

    explicit enumerate_iterator(It iterator)
        : index(std::numeric_limits<size_type>::max())
        , iterator(iterator)
    {
    }

    value_type operator*() const { return { index, *iterator }; }

    bool operator==(const enumerate_iterator& o) const { return iterator == o.iterator; }
    bool operator!=(const enumerate_iterator& o) const { return iterator != o.iterator; }

    enumerate_iterator& operator++()
    {
        iterator++;
        index++;

        return *this;
    }
};

template <typename It, typename size_type>
class reverse_enumerate_iterator {
public:
    using value_type = std::pair<size_type, typename std::iterator_traits<It>::reference>;

private:
    size_type index;
    It iterator;

public:
    reverse_enumerate_iterator(size_type index, It iterator)
        : index(index)
        , iterator(iterator)
    {
    }

    explicit reverse_enumerate_iterator(It iterator)
        : index(std::numeric_limits<size_type>::max())
        , iterator(iterator)
    {
    }

    value_type operator*() const { return { index, *iterator }; }

    bool operator==(const reverse_enumerate_iterator& o) const { return iterator == o.iterator; }
    bool operator!=(const reverse_enumerate_iterator& o) const { return iterator != o.iterator; }

    reverse_enumerate_iterator& operator++()
    {
        iterator++;
        index--;

        return *this;
    }
};

template <typename T>
class range_iterator {
public:
    using value_type = T;

private:
    T i;

public:
    explicit range_iterator(const T v)
        : i(v)
    {
    }

    value_type operator*() const { return i; }

    bool operator==(const range_iterator& o) const { return i == o.i; }
    bool operator!=(const range_iterator& o) const { return i != o.i; }

    range_iterator& operator++()
    {
        i++;
        return *this;
    }
};

template <typename T>
class reverse_range_iterator {
public:
    using value_type = T;

private:
    T i;

public:
    explicit reverse_range_iterator(const T v)
        : i(v)
    {
    }

    value_type operator*() const { return i; }

    bool operator==(const reverse_range_iterator& o) const { return i == o.i; }
    bool operator!=(const reverse_range_iterator& o) const { return i != o.i; }

    reverse_range_iterator& operator++()
    {
        i--;
        return *this;
    }
};

template <typename It>
class iterator_wrapper {
private:
    It beginIt;
    It endIt;

public:
    explicit iterator_wrapper(It b, It e)
        : beginIt(b)
        , endIt(e)
    {
    }

    [[nodiscard]] const It& begin() const { return beginIt; }
    [[nodiscard]] const It& end() const { return endIt; }
};

template <typename C>
auto enumerate(C& c)
{
    using EI = enumerate_iterator<typename C::iterator, typename C::size_type>;
    return iterator_wrapper(EI(0, c.begin()),
                            EI(c.end()));
}

template <typename C>
auto enumerate(const C& c)
{
    using EI = enumerate_iterator<typename C::const_iterator, typename C::size_type>;
    return iterator_wrapper(EI(0, c.cbegin()),
                            EI(c.cend()));
}

template <typename C>
auto const_enumerate(const C& c)
{
    using EI = enumerate_iterator<typename C::const_iterator, typename C::size_type>;
    return iterator_wrapper(EI(0, c.cbegin()),
                            EI(c.cend()));
}

template <typename C>
auto reverse_enumerate(C& c)
{
    using EI = reverse_enumerate_iterator<typename C::reverse_iterator, typename C::size_type>;
    return iterator_wrapper(EI(c.size() - 1, c.rbegin()),
                            EI(c.rend()));
}

template <typename C>
auto reverse_enumerate(const C& c)
{
    using EI = reverse_enumerate_iterator<typename C::const_reverse_iterator, typename C::size_type>;
    return iterator_wrapper(EI(c.size() - 1, c.crbegin()),
                            EI(c.crend()));
}

template <typename C>
auto const_reverse_enumerate(const C& c)
{
    using EI = reverse_enumerate_iterator<typename C::const_reverse_iterator, typename C::size_type>;
    return iterator_wrapper(EI(c.size() - 1, c.crbegin()),
                            EI(c.crend()));
}

inline auto range(const size_t end)
{
    return iterator_wrapper(range_iterator<size_t>(0),
                            range_iterator<size_t>(end));
}

inline auto range(const size_t start, const size_t end)
{
    assert(start <= end);

    return iterator_wrapper(range_iterator<size_t>(start),
                            range_iterator<size_t>(end));
}

inline auto irange(const int end)
{
    return iterator_wrapper(range_iterator<int>(0),
                            range_iterator<int>(end > 0 ? end : 0));
}

inline auto irange(const int start, const int end)
{
    assert(start <= end);

    return iterator_wrapper(range_iterator<int>(start),
                            range_iterator<int>(end));
}

// Output is range(end) but in reverse order
inline auto reverse_range(const size_t end)
{
    return iterator_wrapper(reverse_range_iterator<size_t>(end - 1),
                            reverse_range_iterator<size_t>(-1));
}

// Output is irange(end) but in reverse order
inline auto reverse_irange(const int end)
{
    return iterator_wrapper(reverse_range_iterator<int>(end >= 0 ? end - 1 : -1),
                            reverse_range_iterator<int>(-1));
}

template <class C>
inline auto skip_first_element(const C& c)
{
    auto it = c.cbegin();
    if (it != c.cend()) {
        it++;
    }

    return iterator_wrapper(it, c.cend());
}

template <class C>
inline auto reverse(const C& c)
{
    return iterator_wrapper(c.crbegin(), c.crend());
}
