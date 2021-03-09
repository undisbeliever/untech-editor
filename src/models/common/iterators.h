/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

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

    enumerate_iterator(It iterator)
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

    reverse_enumerate_iterator(It iterator)
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

template <typename It>
class iterator_wrapper {
private:
    const It beginIt;
    const It endIt;

public:
    iterator_wrapper(It b, It e)
        : beginIt(b)
        , endIt(e)
    {
    }

    const It& begin() const { return beginIt; }
    const It& end() const { return endIt; }
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
