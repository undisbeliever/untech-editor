/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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

class ListItemError : public AbstractSpecializedError {
    const void* _ptr;
    const std::string _message;

public:
    template <typename... Args>
    ListItemError(const void* ptr, Args... message)
        : _ptr(ptr)
        , _message(stringBuilder(message...))
    {
    }
    virtual ~ListItemError() = default;

    virtual std::string message() const final;

    const void* ptr() const { return _ptr; }
};

struct ErrorListItem {
    enum class ErrorType {
        WARNING,
        ERROR
    };

    ErrorType type;
    std::string message;
    std::unique_ptr<const AbstractSpecializedError> specialized;
};

class ErrorList {
public:
    using ErrorType = ErrorListItem::ErrorType;

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
        _list.push_back({ ErrorType::ERROR, {}, std::move(e) });
        _errorCount++;
    }
    void addErrorString(const std::string& s)
    {
        _list.push_back({ ErrorType::ERROR, s, nullptr });
        _errorCount++;
    }
    void addErrorString(std::string&& s)
    {
        _list.push_back({ ErrorType::ERROR, std::move(s), nullptr });
        _errorCount++;
    }
    template <typename... Args>
    void addErrorString(const Args... args)
    {
        _list.push_back({ ErrorType::ERROR, stringBuilder(args...), nullptr });
        _errorCount++;
    }

    void addWarning(std::unique_ptr<const AbstractSpecializedError> e)
    {
        _list.push_back({ ErrorType::WARNING, {}, std::move(e) });
    }
    template <typename... Args>
    void addWarningString(const Args... args)
    {
        _list.push_back(ErrorListItem{ ErrorType::WARNING, stringBuilder(args...), nullptr });
    }

    void printIndented(std::ostream& out) const;
};

}
