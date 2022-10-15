/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <string>

namespace UnTech {

class StringParser {
private:
    const std::u8string _inputString;
    std::u8string::const_iterator _pos;
    unsigned _lineNo;

public:
    static bool isWhitespaceChar(const std::u8string::value_type c);

    StringParser(std::u8string&& str);

    void reset();

    [[nodiscard]] std::u8string::const_iterator pos() const { return _pos; }
    [[nodiscard]] std::u8string::const_iterator end() const { return _inputString.end(); }
    [[nodiscard]] unsigned lineNo() const { return _lineNo; }

    [[nodiscard]] bool atEnd() const;

    // returns true if the current char is a whitespace character
    [[nodiscard]] bool isWhitespace() const;

    // returns char 0 if we cannot peek at the next char
    [[nodiscard]] std::u8string::value_type cur() const;

    // returns char 0 if cannot peek at next char
    [[nodiscard]] std::u8string::value_type peek() const;

    // advances to the next char and returns it
    void advance();

    void skipWhitespace();

    bool testAndConsume(const std::u8string_view str);

    // returns false if the string ended unexpectedly
    // lineNo is not incremented if the string ends unexpectedly
    bool skipUntil(const std::u8string_view str);
    bool skipUntil(const std::u8string::value_type c);
};

}
