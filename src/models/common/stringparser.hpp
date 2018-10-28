/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

/*
 * This implementation is a seperate file to improve compile times.
 */

#pragma once

#include "stringparser.h"
#include <algorithm>

namespace UnTech {

inline bool StringParser::isWhitespaceChar(const std::string::value_type c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline StringParser::StringParser(const std::string& str)
    : _inputString(str)
    , _pos(_inputString.cbegin())
    , _lineNo(0)
{
    reset();
}

inline StringParser::StringParser(const std::string&& str)
    : _inputString(std::move(str))
    , _pos(_inputString.cbegin())
    , _lineNo(0)
{
    reset();
}

inline void StringParser::reset()
{
    _pos = _inputString.cbegin();
    _lineNo = 1;
}

inline bool StringParser::atEnd() const
{
    return _pos == _inputString.cend();
}

inline bool StringParser::isWhitespace() const
{
    return _pos != _inputString.cend() && isWhitespaceChar(*_pos);
}

inline std::string::value_type StringParser::cur() const
{
    if (_pos != _inputString.cend()) {
        return *_pos;
    }
    else {
        return 0;
    }
}

inline std::string::value_type StringParser::peek() const
{
    if (_pos == _inputString.cend()) {
        return 0;
    }
    auto nextIt = _pos + 1;
    if (nextIt == _inputString.cend()) {
        return 0;
    }
    else {
        return *nextIt;
    }
}

inline void StringParser::advance()
{
    if (_pos != _inputString.cend()) {
        _pos++;

        if (_pos != _inputString.cend()) {
            if (*_pos == '\n') {
                _lineNo++;
            }
        }
    }
}

inline void StringParser::skipWhitespace()
{
    while (_pos != _inputString.cend() && isWhitespaceChar(*_pos)) {
        if (*_pos == '\n') {
            _lineNo++;
        }
        _pos++;
    }
}

inline bool StringParser::testAndConsume(const std::string& str)
{
    std::string::const_iterator it = _pos;
    for (auto c : str) {
        if (it == _inputString.cend()) {
            return false;
        }
        if (*it != c) {
            return false;
        }
        it++;
    }

    _lineNo += std::count(str.cbegin(), str.cend(), '\n');
    _pos = it;

    return true;
}

inline bool StringParser::skipUntil(const std::string& str)
{
    auto oldPos = _pos;

    auto it = std::search(_pos, _inputString.cend(),
                          str.begin(), str.end());

    if (it != _inputString.cend()) {
        _pos = it + str.size();
        _lineNo += std::count(oldPos, _pos, '\n');
        return true;
    }
    else {
        _pos = _inputString.cend();
        return false;
    }
}

inline bool StringParser::skipUntil(const std::string::value_type c)
{
    auto oldPos = _pos;

    auto it = std::find(_pos, _inputString.cend(), c);
    if (it != _inputString.cend()) {
        _pos = it + 1;
        _lineNo += std::count(oldPos, _pos, '\n');
        return true;
    }
    else {
        _pos = _inputString.cend();
        return false;
    }
}

}
