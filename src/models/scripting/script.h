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

struct Statement;
struct IfStatement;
struct WhileStatement;
struct Comment;

// ::TODO add CustomIfStatement ::
// NOTE: If I change ScriptNode then I must also update UnTech::Gui::RoomEditorData::AP::ScriptStatements::getList();
using ScriptNode = std::variant<Statement, IfStatement, WhileStatement, Comment>;

enum class ConditionalType {
    Word,
    Flag,
};

enum class ComparisonType {
    Equal,
    NotEqual,
    LessThan,
    GreaterThanEqual,

    Set,
    Clear,
};

struct Conditional {
    ConditionalType type;
    idstring variable;
    ComparisonType comparison;
    std::string value;

    bool operator==(const Conditional& o) const
    {
        return type == o.type
               && variable == o.variable
               && comparison == o.comparison
               && value == o.value;
    }
};

struct Statement {
    idstring opcode;
    std::array<std::string, 2> arguments;

    bool operator==(const Statement& o) const
    {
        return opcode == o.opcode
               && arguments == o.arguments;
    }
};

struct Comment {
    std::string text;

    bool operator==(const Comment& o) const
    {
        return text == o.text;
    }
};

struct IfStatement {
    Conditional condition;
    std::vector<ScriptNode> thenStatements;
    std::vector<ScriptNode> elseStatements;

    bool operator==(const IfStatement& o) const;
};

struct WhileStatement {
    Conditional condition;
    std::vector<ScriptNode> statements;

    bool operator==(const WhileStatement& o) const;
};

struct Script {
    constexpr static unsigned MAX_DEPTH = 8;

    idstring name;
    // ::TODO add local flags and words::

    std::vector<ScriptNode> statements;

    bool operator==(const Script& o) const;
};

inline bool IfStatement::operator==(const IfStatement& o) const
{
    return condition == o.condition
           && thenStatements == o.thenStatements
           && elseStatements == o.elseStatements;
}

inline bool WhileStatement::operator==(const WhileStatement& o) const
{
    return condition == o.condition
           && statements == o.statements;
}

inline bool Script::operator==(const Script& o) const
{
    return name == o.name
           && statements == o.statements;
}

}
