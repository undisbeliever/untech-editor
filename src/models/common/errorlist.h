/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "stringbuilder.h"
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace UnTech {

class StringStream;

class AbstractError {
public:
    const std::u8string message;
    bool isWarning{ false };

public:
    AbstractError(std::u8string&& msg)
        : message(std::move(msg))

    {
    }

    virtual ~AbstractError() = default;

    // Error displayed in command line compiler
    // By default prints message
    // The first line is already indented, you MUST ident all subsequent lines
    // You SHOULD NOT end the steam on a new line
    virtual void printIndented(StringStream& out) const;
};

template <typename T>
requires std::is_enum_v<T>
struct GenericListError : public AbstractError {
public:
    const T type;
    const unsigned firstIndex;
    const unsigned childIndex;

    GenericListError(T type_, unsigned pIndex, std::u8string message)
        : AbstractError{ std::move(message) }
        , type{ type_ }
        , firstIndex{ pIndex }
        , childIndex{ 0 }
    {
    }

    GenericListError(T type_, unsigned pIndex, unsigned cIndex, std::u8string message)
        : AbstractError{ std::move(message) }
        , type{ type_ }
        , firstIndex{ pIndex }
        , childIndex{ cIndex }
    {
    }

    ~GenericListError() override = default;
};

class ErrorList {
private:
    std::vector<std::unique_ptr<AbstractError>> _list;
    unsigned _errorCount;

public:
    ErrorList();

    [[nodiscard]] const std::vector<std::unique_ptr<AbstractError>>& list() const { return _list; }

    [[nodiscard]] inline bool empty() const { return _list.empty(); }
    [[nodiscard]] unsigned errorCount() const { return _errorCount; }
    [[nodiscard]] unsigned warningCount() const { return _list.size() - _errorCount; }

    [[nodiscard]] bool hasError() const { return _errorCount > 0; }

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
