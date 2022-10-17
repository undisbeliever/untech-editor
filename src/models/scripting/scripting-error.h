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
using BytecodeError = GenericListError<BytecodeErrorType>;

enum class GameStateErrorType {
    FLAG,
    WORD,
};
using GameStateError = GenericListError<GameStateErrorType>;

enum class ScriptErrorType {
    TEMP_FLAG,
    TEMP_WORD,

    SCRIPT,
    SCRIPT_LINE,
};
using ScriptError = GenericListError<ScriptErrorType>;

}
