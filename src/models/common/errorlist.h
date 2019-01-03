/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

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

class ErrorList {
public:
    enum class ErrorType {
        WARNING,
        ERROR
    };

    struct ErrorItem {
        ErrorType type;
        std::string message;
        std::unique_ptr<const AbstractSpecializedError> specialized;
    };

private:
    std::vector<ErrorItem> _list;
    unsigned _errorCount;

public:
    ErrorList();

    const std::vector<ErrorItem>& list() const { return _list; }

    inline bool empty() const { return _list.empty(); }
    unsigned errorCount() const { return _errorCount; }
    unsigned warningCount() const { return _list.size() - _errorCount; }

    bool hasError() const { return _errorCount > 0; }

    void addError(const std::string& s)
    {
        _list.push_back({ ErrorType::ERROR, s, nullptr });
        _errorCount++;
    }
    void addError(std::string&& s)
    {
        _list.push_back({ ErrorType::ERROR, std::move(s), nullptr });
        _errorCount++;
    }
    void addError(std::unique_ptr<const AbstractSpecializedError> e)
    {
        _list.push_back({ ErrorType::ERROR, {}, std::move(e) });
        _errorCount++;
    }

    void addWarning(const std::string& s)
    {
        _list.push_back({ ErrorType::WARNING, s, nullptr });
    }
    void addWarning(std::string&& s)
    {
        _list.push_back({ ErrorType::WARNING, std::move(s), nullptr });
    }
    void addWarning(std::unique_ptr<const AbstractSpecializedError> e)
    {
        _list.push_back({ ErrorType::WARNING, {}, std::move(e) });
    }

    void printIndented(std::ostream& out) const;
};

}
