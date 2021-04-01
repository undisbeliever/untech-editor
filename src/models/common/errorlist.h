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

class AbstractSpecializedError {
public:
    virtual ~AbstractSpecializedError() = default;

    // Error displayed in GUI.
    virtual std::string message() const = 0;

    // Error displayed in command line compiler
    // By default prints message()
    // The first line is already indented, you MUST ident all subsequent lines
    // You SHOULD NOT end the steam on a new line
    virtual void printIndented(std::ostream& out) const;
};

struct GenericListError : public AbstractSpecializedError {
public:
    const unsigned firstIndex;
    const unsigned childIndex;
    const std::string msg;

    GenericListError(unsigned pIndex, std::string&& message)
        : firstIndex(pIndex)
        , childIndex(0)
        , msg(std::move(message))
    {
    }

    GenericListError(unsigned pIndex, unsigned cIndex, std::string&& message)
        : firstIndex(pIndex)
        , childIndex(cIndex)
        , msg(std::move(message))
    {
    }

    virtual ~GenericListError() = default;

    virtual std::string message() const final;
};

struct ErrorListItem {
    std::string message;
    std::unique_ptr<const AbstractSpecializedError> specialized;
    bool isWarning;
};

class ErrorList {
private:
    std::vector<ErrorListItem> _list;
    unsigned _errorCount;

public:
    ErrorList();

    const std::vector<ErrorListItem>& list() const { return _list; }

    inline bool empty() const { return _list.empty(); }
    unsigned errorCount() const { return _errorCount; }
    unsigned warningCount() const { return _list.size() - _errorCount; }

    bool hasError() const { return _errorCount > 0; }

    void addError(std::unique_ptr<const AbstractSpecializedError> e)
    {
        _list.push_back({ e->message(), std::move(e), false });
        _errorCount++;
    }
    void addErrorString(const std::string& s)
    {
        _list.push_back({ s, nullptr, false });
        _errorCount++;
    }
    void addErrorString(std::string&& s)
    {
        _list.push_back({ std::move(s), nullptr, false });
        _errorCount++;
    }
    template <typename... Args>
    void addErrorString(const Args... args)
    {
        _list.push_back({ stringBuilder(args...), nullptr, false });
        _errorCount++;
    }

    void addWarning(std::unique_ptr<const AbstractSpecializedError> e)
    {
        _list.push_back({ e->message(), std::move(e), true });
    }
    template <typename... Args>
    void addWarningString(const Args... args)
    {
        _list.push_back(ErrorListItem{ stringBuilder(args...), nullptr, true });
    }

    void printIndented(std::ostream& out) const;
};

}
