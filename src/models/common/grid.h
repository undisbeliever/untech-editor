/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "aabb.h"
#include "exceptions.h"
#include "stringbuilder.h"
#include "models/common/iterators.h"
#include <cassert>
#include <span>
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

    grid(unsigned width, unsigned height, container&& data)
        : _width((width > 0 && height > 0) ? width : 0)
        , _height((width > 0 && height > 0) ? height : 0)
        , _grid(std::move(data))
    {
        if (_grid.size() != _width * _height) {
            throw invalid_argument("grid data size must equal width * height");
        }
    }

    ~grid() = default;
    grid(const grid&) = default;
    grid(grid&&) = default;
    grid& operator=(const grid&) = default;
    grid& operator=(grid&&) = default;

    bool empty() const { return _width == 0 || _height == 0; }

    unsigned width() const { return _width; }
    unsigned height() const { return _height; }
    unsigned cellCount() const { return _grid.size(); }

    const container& gridData() const { return _grid; }

    usize size() const { return usize(_width, _height); }

    std::span<T> scanline(unsigned y)
    {
        if (y >= _height) {
            throw out_of_range("grid::scanline out of range");
        }
        return std::span(_grid.data() + (y * _width), _width);
    }

    std::span<const T> scanline(unsigned y) const
    {
        if (y >= _height) {
            throw out_of_range("grid::scanline out of range");
        }
        return std::span(_grid.data() + (y * _width), _width);
    }

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

    size_t cellPos(unsigned x, unsigned y) const
    {
        _rangeCheck(x, y);
        return y * _width + x;
    }
    size_t cellPos(const upoint& p) const { return at(p.x, p.y); }

    inline void set(unsigned x, unsigned y, const T& value)
    {
        _rangeCheck(x, y);
        _grid.at(y * _width + x) = value;
    }
    inline void set(const upoint& p, const T& value) { set(p.x, p.y, value); }

    void setCells(const unsigned xPos, const unsigned yPos, const grid& values)
    {
        _rangeCheck(xPos, yPos, values.width(), values.height());

        const auto startIt = _grid.begin() + yPos * _width + xPos;
        auto vIt = values.cbegin();
        for (const auto y : range(values.height())) {
            auto gridIt = startIt + _width * y;

            for (const auto x : range(values.width())) {
                gridIt[x] = *vIt++;
            }
        }
        assert(vIt == values.cend());
    }
    inline void setCells(const upoint& pos, const grid& values)
    {
        return setCells(pos.x, pos.y, values);
    }

    grid subGrid(unsigned xPos, unsigned yPos, unsigned sgWidth, unsigned sgHeight) const
    {
        _rangeCheck(xPos, yPos, sgWidth, sgHeight);

        grid ret(sgWidth, sgHeight);
        if (ret.empty()) {
            return ret;
        }

        const auto startIt = _grid.cbegin() + yPos * _width + xPos;
        auto retIt = ret.begin();

        for (const auto y : range(sgHeight)) {
            auto gridIt = startIt + _width * y;
            for (const auto x : range(sgWidth)) {
                *retIt++ = gridIt[x];
            }
        }
        assert(retIt == ret.end());

        return ret;
    }
    inline grid subGrid(const upoint& pos, const usize& size) const
    {
        return subGrid(pos.x, pos.y, size.width, size.height);
    }
    inline grid subGrid(const urect& r) const
    {
        return subGrid(r.x, r.y, r.width, r.height);
    }

    // Returns a NEW grid, resized to newSize with all new cells containing value
    grid resized(const unsigned newWidth, const unsigned newHeight, const T& value) const
    {
        if (_width == newWidth && _height == newHeight) {
            return *this;
        }

        grid ret(newWidth, newHeight, value);

        unsigned widthToCopy = std::min(newWidth, _width);
        unsigned heightToCopy = std::min(newHeight, _height);

        for (const auto y : range(heightToCopy)) {
            auto gridIt = this->_grid.cbegin() + this->_width * y;
            auto retIt = ret._grid.begin() + ret._width * y;

            for (const auto x : range(widthToCopy)) {
                retIt[x] = gridIt[x];
            }
        }

        return ret;
    }
    inline grid resized(const usize& newSize, const T& value) const
    {
        return resized(newSize.width, newSize.height, value);
    }

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

    bool operator==(const grid<T>&) const = default;

private:
    void _rangeCheck(unsigned x, unsigned y) const
    {
        if (x >= _width) {
            throw out_of_range("grid: x (", x, ") >= width (", _width, ")");
        }
        if (y >= _height) {
            throw out_of_range("grid: y (", y, ") >= height (", _height, ")");
        }
    }
    void _rangeCheck(unsigned x, unsigned y, unsigned width, unsigned height) const
    {
        _rangeCheck(x, y);
        if (x + width > _width) {
            throw out_of_range("grid: x + width (", (x + width), ") > width (", _width, ")");
        }
        if (y + height > _height) {
            throw out_of_range("grid: y + height(", (y + height), ") > height (", _height, ")");
        }
    }
};
}
