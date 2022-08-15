/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include <array>
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
    std::u8string value;

    bool operator==(const Conditional&) const = default;
};

struct Statement {
    idstring opcode;
    std::array<std::u8string, 2> arguments;

    bool operator==(const Statement&) const = default;
};

struct Comment {
    std::u8string text;

    bool operator==(const Comment&) const = default;
};

struct IfStatement {
    Conditional condition;
    std::vector<ScriptNode> thenStatements;
    std::vector<ScriptNode> elseStatements;

    // Cannot use default here, ScriptNode is incomplete.
    bool operator==(const IfStatement& o) const;
};

struct WhileStatement {
    Conditional condition;
    std::vector<ScriptNode> statements;

    // Cannot use default here, ScriptNode is incomplete.
    bool operator==(const WhileStatement& o) const;
};

struct Script {
    constexpr static unsigned MAX_DEPTH = 8;

    idstring name;

    std::vector<ScriptNode> statements;

    bool operator==(const Script&) const = default;
};

struct RoomScripts {
    RoomScripts();

    std::vector<idstring> tempFlags;
    std::vector<idstring> tempWords;

    Scripting::Script startupScript;

    NamedList<Scripting::Script> scripts;

    bool operator==(const RoomScripts&) const = default;
};

extern const idstring STARTUP_SCRIPT_NAME;

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

}
