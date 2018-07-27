/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "aabb.h"
#include <stdexcept>
#include <vector>

namespace UnTech {

template <typename T>
class grid {
    using container = typename std::vector<T>;

    using size_type = typename container::size_type;
    using iterator = typename container::iterator;
    using reverse_iterator = typename container::reverse_iterator;
    using const_iterator = typename container::const_iterator;
    using const_reverse_iterator = typename container::const_reverse_iterator;

private:
    unsigned _width = 0;
    unsigned _height = 0;
    container _grid;

public:
    explicit grid()
        : grid(0, 0)
    {
    }

    grid(unsigned width, unsigned height, const T& def = T())
        : _width((width > 0 && height > 0) ? width : 0)
        , _height((width > 0 && height > 0) ? height : 0)
        , _grid(_width * _height, def)
    {
    }

    grid(const usize& size, const T& def = T())
        : grid(size.width, size.height, def)
    {
    }

    ~grid() = default;
    grid(const grid&) = default;
    grid(grid&&) = default;
    grid& operator=(const grid&) = default;
    grid& operator=(grid&&) = default;

    bool empty() const { return _width == 0 || _height == 0; }

    unsigned width() const { return _width; }
    unsigned height() const { return _height; }
    unsigned gridSize() const { return _grid.size(); }

    usize size() const { return usize(_width, _height); }

    T& at(unsigned x, unsigned y)
    {
        _rangeCheck(x, y);
        return _grid.at(y * _width + x);
    }
    T& at(const upoint& p) { return at(p.x, p.y); }

    const T& at(unsigned x, unsigned y) const
    {
        _rangeCheck(x, y);
        return _grid.at(y * _width + x);
    }
    const T& at(const upoint& p) const { return at(p.x, p.y); }

    inline void set(unsigned x, unsigned y, const T& value)
    {
        _rangeCheck(x, y);
        _grid.at(y * _width + x) = value;
    }
    inline void set(const upoint& p, const T& value) { set(p.x, p.y, value); }

    iterator begin() { return _grid.begin(); }
    iterator end() { return _grid.end(); }
    reverse_iterator rbegin() { return _grid.rbegin(); }
    reverse_iterator rend() { return _grid.rend(); }

    const_iterator begin() const { return _grid.cbegin(); }
    const_iterator end() const { return _grid.cend(); }
    const_reverse_iterator rbegin() const { return _grid.crbegin(); }
    const_reverse_iterator rend() const { return _grid.crend(); }

    const_iterator cbegin() const { return _grid.cbegin(); }
    const_iterator cend() const { return _grid.cend(); }
    const_reverse_iterator crbegin() const { return _grid.crbegin(); }
    const_reverse_iterator crend() const { return _grid.crend(); }

    bool operator==(const grid<T>& o) const
    {
        return _width == o._width
               && _height == o._height
               && _grid == o._grid;
    }
    bool operator!=(const grid<T>& o) const { return !(*this == o); }

private:
    void _rangeCheck(unsigned x, unsigned y) const
    {
        if (x >= _width) {
            throw std::range_error("grid: x (" + std::to_string(x) + ") >= width (" + std::to_string(_width) + ")");
        }
        if (y >= _height) {
            throw std::range_error("grid: y (" + std::to_string(y) + ") >= height (" + std::to_string(_height) + ")");
        }
    }
};
}
