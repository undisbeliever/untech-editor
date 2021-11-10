/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"

namespace UnTech::Scripting {

enum class BytecodeErrorType {
    INSTRUCTION,
};

class BytecodeError : public GenericListError {
public:
    const BytecodeErrorType type;

    BytecodeError(const BytecodeErrorType type, unsigned index, std::u8string&& message)
        : GenericListError(index, std::move(message))
        , type(type)
    {
    }
};

enum class GameStateErrorType {
    FLAG,
    WORD,
};

class GameStateError : public GenericListError {
public:
    const GameStateErrorType type;

    GameStateError(const GameStateErrorType type, unsigned index, std::u8string&& message)
        : GenericListError(index, std::move(message))
        , type(type)
    {
    }
};

enum class ScriptErrorType {
    TEMP_FLAG,
    TEMP_WORD,

    SCRIPT,
    SCRIPT_LINE,
};

class ScriptError : public GenericListError {
public:
    const ScriptErrorType type;

    ScriptError(const ScriptErrorType type, unsigned index, std::u8string&& message)
        : GenericListError(index, std::move(message))
        , type(type)
    {
    }

    ScriptError(const ScriptErrorType type, unsigned index, unsigned childIndex, std::u8string&& message)
        : GenericListError(index, childIndex, std::move(message))
        , type(type)
    {
    }
};

}
