/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include <variant>

namespace UnTech::Scripting {

struct BytecodeMapping;
struct GameStateData;

struct Statement {
    idstring opcode;
    std::array<std::string, 2> arguments;

    bool operator==(const Statement& o) const
    {
        return opcode == o.opcode
               && arguments == o.arguments;
    }
};

// ::TODO add IfStatement and WhileStatement and CustomIfStatement ::
using ScriptNode = std::variant<Statement>;

struct Script {
    constexpr static unsigned MAX_DEPTH = 8;

    idstring name;
    // ::TODO add local flags and words::

    std::vector<ScriptNode> statements;

    bool operator==(const Script& o) const
    {
        return name == o.name
               && statements == o.statements;
    }
};

}
