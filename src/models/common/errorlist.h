/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "stringbuilder.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {

class StringStream;

class AbstractError {
public:
    const std::u8string message;
    bool isWarning;

public:
    AbstractError(std::u8string&& msg)
        : message(std::move(msg))
        , isWarning(false)
    {
    }

    virtual ~AbstractError() = default;

    // Error displayed in command line compiler
    // By default prints message
    // The first line is already indented, you MUST ident all subsequent lines
    // You SHOULD NOT end the steam on a new line
    virtual void printIndented(StringStream& out) const;
};

struct GenericListError : public AbstractError {
public:
    const unsigned firstIndex;
    const unsigned childIndex;

    GenericListError(unsigned pIndex, std::u8string&& message)
        : AbstractError(std::move(message))
        , firstIndex(pIndex)
        , childIndex(0)
    {
    }

    GenericListError(unsigned pIndex, unsigned cIndex, std::u8string&& message)
        : AbstractError(std::move(message))
        , firstIndex(pIndex)
        , childIndex(cIndex)
    {
    }

    virtual ~GenericListError() = default;
};

class ErrorList {
private:
    std::vector<std::unique_ptr<AbstractError>> _list;
    unsigned _errorCount;

public:
    ErrorList();

    const std::vector<std::unique_ptr<AbstractError>>& list() const { return _list; }

    inline bool empty() const { return _list.empty(); }
    unsigned errorCount() const { return _errorCount; }
    unsigned warningCount() const { return _list.size() - _errorCount; }

    bool hasError() const { return _errorCount > 0; }

    void addError(std::unique_ptr<AbstractError> e)
    {
        e->isWarning = false;
        _list.push_back(std::move(e));

        _errorCount++;
    }

    template <typename... Args>
    void addErrorString(Args... args)
    {
        addError(std::make_unique<AbstractError>(stringBuilder(std::forward<Args>(args)...)));
    }

    void addWarning(std::unique_ptr<AbstractError> e)
    {
        e->isWarning = true;
        _list.push_back(std::move(e));
    }

    template <typename... Args>
    void addWarningString(Args... args)
    {
        addWarning(std::make_unique<AbstractError>(stringBuilder(std::forward<Args>(args)...)));
    }

    void printIndented(StringStream& out) const;
};

}
