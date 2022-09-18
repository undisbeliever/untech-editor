/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "scripting-serializer.h"
#include "models/common/enummap.h"
#include "models/common/iterators.h"

namespace UnTech::Scripting {

using namespace std::string_literals;

static const EnumMap<ArgumentType> argumentTypeEnumMap = {
    { u8"unused", ArgumentType::Unused },
    { u8"flag", ArgumentType::Flag },
    { u8"word", ArgumentType::Word },
    { u8"immediate-u16", ArgumentType::ImmediateU16 },
    { u8"room-script", ArgumentType::RoomScript },
    { u8"entity-group", ArgumentType::EntityGroup },
    { u8"room", ArgumentType::Room },
    { u8"room-entrance", ArgumentType::RoomEntrance },
};

static const EnumMap<ConditionalType> conditionalTypeMap = {
    { u8"word", ConditionalType::Word },
    { u8"flag", ConditionalType::Flag },
};

static const EnumMap<ComparisonType> comparisonTypeMap = {
    { u8"==", ComparisonType::Equal },
    { u8"!=", ComparisonType::NotEqual },
    { u8"&lt;", ComparisonType::LessThan },
    { u8"&gt;=", ComparisonType::GreaterThanEqual },
    { u8"set", ComparisonType::Set },
    { u8"clear", ComparisonType::Clear },
};

void readGameState(GameState& gameState, Xml::XmlReader& xml, const Xml::XmlTag& tag)
{
    assert(tag.name == u8"game-state");
    assert(gameState.flags.empty());
    assert(gameState.words.empty());

    gameState.startingRoom = tag.getAttributeOptionalId(u8"starting-room");
    gameState.startingEntrance = tag.getAttributeOptionalId(u8"starting-entrance");
    gameState.startingPlayer = tag.getAttributeOptionalId(u8"starting-player");

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == u8"flag") {
            const unsigned index = childTag.getAttributeUnsigned(u8"index", 0, GameState::MAX_FLAGS * 2);

            if (index >= gameState.flags.size()) {
                gameState.flags.resize(index + 1);
            }
            auto& f = gameState.flags.at(index);

            f.name = childTag.getAttributeOptionalId(u8"name");
            f.room = childTag.getAttributeOptionalId(u8"room");
        }
        else if (childTag.name == u8"word") {
            const unsigned index = childTag.getAttributeUnsigned(u8"index", 0, GameState::MAX_FLAGS * 2);

            if (index >= gameState.words.size()) {
                gameState.words.resize(index + 1);
            }
            auto& w = gameState.words.at(index);

            w.name = childTag.getAttributeOptionalId(u8"name");
            w.room = childTag.getAttributeOptionalId(u8"room");

            if (childTag.hasAttribute(u8"value")) {
                w.initialValue = childTag.getAttributeUint16(u8"value");
            }
        }
        else {
            throw Xml::unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }
}

void writeGameState(Xml::XmlWriter& xml, const GameState& gameState)
{
    xml.writeTag(u8"game-state");

    xml.writeTagAttribute(u8"starting-room", gameState.startingRoom);
    xml.writeTagAttribute(u8"starting-entrance", gameState.startingEntrance);
    xml.writeTagAttribute(u8"starting-player", gameState.startingPlayer);

    for (auto [i, f] : const_enumerate(gameState.flags)) {
        if (f.name.isValid()) {
            xml.writeTag(u8"flag");
            xml.writeTagAttribute(u8"index", unsigned(i));
            xml.writeTagAttributeOptional(u8"name", f.name);
            xml.writeTagAttributeOptional(u8"room", f.room);
            xml.writeCloseTag();
        }
    }

    for (auto [i, w] : const_enumerate(gameState.words)) {
        if (w.name.isValid()) {
            xml.writeTag(u8"word");
            xml.writeTagAttribute(u8"index", unsigned(i));
            xml.writeTagAttributeOptional(u8"name", w.name);
            xml.writeTagAttributeOptional(u8"room", w.room);
            xml.writeTagAttribute(u8"value", w.initialValue);
            xml.writeCloseTag();
        }
    }

    xml.writeCloseTag();
}

void readBytecode(BytecodeInput& bytecode, Xml::XmlReader& xml, const Xml::XmlTag& tag)
{
    assert(tag.name == u8"bytecode");
    assert(bytecode.instructions.empty());

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == u8"instruction") {
            Instruction& inst = bytecode.instructions.emplace_back();

            inst.name = childTag.getAttributeOptionalId(u8"name");

            auto readArg = [&](const std::u8string& name) {
                return childTag.getAttributeOptionalEnum(name, argumentTypeEnumMap, ArgumentType::Unused);
            };
            inst.arguments.at(0) = readArg(u8"arg1");
            inst.arguments.at(1) = readArg(u8"arg2");
            assert(inst.arguments.size() == 2);

            inst.yields = childTag.getAttributeBoolean(u8"yields");
        }
        else {
            throw Xml::unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }
}

void writeBytecode(Xml::XmlWriter& xml, const BytecodeInput& bytecode)
{
    using namespace std::string_literals;

    xml.writeTag(u8"bytecode");

    for (const auto& inst : bytecode.instructions) {
        xml.writeTag(u8"instruction");
        xml.writeTagAttributeOptional(u8"name", inst.name);

        auto writeArg = [&](const std::u8string& name, ArgumentType arg) {
            if (arg != ArgumentType::Unused) {
                xml.writeTagAttributeEnum(name, arg, argumentTypeEnumMap);
            }
        };
        writeArg(u8"arg1"s, inst.arguments.at(0));
        writeArg(u8"arg2"s, inst.arguments.at(1));
        assert(inst.arguments.size() == 2);

        xml.writeTagAttribute(u8"yields", inst.yields);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

static const std::array<std::u8string, 2> attributeTagNames = {
    u8"arg1"s,
    u8"arg2"s,
};

static void readWhileTag(WhileStatement& s, Xml::XmlReader& xml);
static void readIfTag(IfStatement& s, Xml::XmlReader& xml);
static void readElseTag(std::vector<ScriptNode>& nodes, Xml::XmlReader& xml);

static void readScriptNode(std::vector<ScriptNode>& nodes, Xml::XmlReader& xml, const Xml::XmlTag& tag)
{
    if (tag.name == u8"statement"s) {
        auto& s = std::get<Statement>(nodes.emplace_back(Statement{}));

        s.opcode = tag.getAttributeOptionalId(u8"opcode"s);
        for (auto [i, arg] : enumerate(s.arguments)) {
            arg = tag.getAttributeOrEmpty(attributeTagNames.at(i));
        }
    }
    else if (tag.name == u8"if"s) {
        auto& s = std::get<IfStatement>(nodes.emplace_back(IfStatement{}));
        readIfTag(s, xml);
    }
    else if (tag.name == u8"else"s) {
        readElseTag(nodes, xml);
    }
    else if (tag.name == u8"while"s) {
        auto& s = std::get<WhileStatement>(nodes.emplace_back(WhileStatement{}));
        readWhileTag(s, xml);
    }
    else if (tag.name == u8"comment") {
        auto& c = std::get<Comment>(nodes.emplace_back(Comment{}));
        c.text = tag.getAttributeOrEmpty(u8"c");
    }
    else if (tag.name == u8"condition") {
        throw Xml::xml_error(tag, u8"<condition> tag not allowed here");
    }
    else {
        throw Xml::unknown_tag_error(tag);
    }
}

static void readScriptNodes(std::vector<ScriptNode>& nodes, Xml::XmlReader& xml)
{
    while (const auto tag = xml.parseTag()) {
        readScriptNode(nodes, xml, tag);
        xml.parseCloseTag();
    }
}

static void readConditionTag(Conditional& c, const Xml::XmlTag& tag)
{
    assert(tag.name == u8"condition");

    c.type = tag.getAttributeOptionalEnum(u8"type", conditionalTypeMap, ConditionalType::Flag);
    c.variable = tag.getAttributeOptionalId(u8"var");
    c.comparison = tag.getAttributeOptionalEnum(u8"comp", comparisonTypeMap, ComparisonType::Set);
    c.value = tag.getAttributeOrEmpty(u8"value");
}

static void readWhileTag(WhileStatement& s, Xml::XmlReader& xml)
{
    // The first tag should be a condition tag
    if (const auto tag = xml.parseTag()) {
        if (tag.name == u8"condition") {
            readConditionTag(s.condition, tag);
        }
        else {
            readScriptNode(s.statements, xml, tag);
        }

        xml.parseCloseTag();
    }

    readScriptNodes(s.statements, xml);
}

static void readIfTag(IfStatement& s, Xml::XmlReader& xml)
{
    // The first tag should be a condition tag
    if (const auto tag = xml.parseTag()) {
        if (tag.name == u8"condition") {
            readConditionTag(s.condition, tag);
        }
        else {
            readScriptNode(s.thenStatements, xml, tag);
        }

        xml.parseCloseTag();
    }

    readScriptNodes(s.thenStatements, xml);
}

static void readElseTag(std::vector<ScriptNode>& nodes, Xml::XmlReader& xml)
{
    struct V {
        Xml::XmlReader& xml;

        explicit V(Xml::XmlReader& x)
            : xml(x)
        {
        }

        void throwError()
        {
            throw Xml::xml_error(xml, u8"<else> tag is not allowed here");
        }

        void operator()(IfStatement& s)
        {
            readScriptNodes(s.elseStatements, xml);
        }

        void operator()(WhileStatement&) { throwError(); }
        void operator()(Statement&) { throwError(); }
        void operator()(Comment&) { throwError(); }
    };

    if (!nodes.empty()) {
        std::visit(V(xml), nodes.back());
    }
    else {
        throw Xml::xml_error(xml, u8"<else> tag is not allowed here");
    }
}

void readScript(RoomScripts& roomScripts, Xml::XmlReader& xml, const Xml::XmlTag& tag)
{
    assert(tag.name == u8"script"s);

    Script script;

    script.name = tag.getAttributeOptionalId(u8"name"s);
    readScriptNodes(script.statements, xml);

    if (script.name != STARTUP_SCRIPT_NAME) {
        roomScripts.scripts.insert_back(std::move(script));
    }
    else {
        roomScripts.startupScript = std::move(script);
    }
}

void readTempScriptVariables(RoomScripts& roomScripts, Xml::XmlReader& xml, const Xml::XmlTag& tag)
{
    assert(tag.name == u8"temp-script-variables"s);

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == u8"flag") {
            roomScripts.tempFlags.push_back(childTag.getAttributeId(u8"name"));
        }
        else if (childTag.name == u8"word") {
            roomScripts.tempWords.push_back(childTag.getAttributeId(u8"name"));
        }
        else {
            throw Xml::unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }
}

static void writeTempScriptVariablesVector(Xml::XmlWriter& xml, const std::u8string& tagName, const std::vector<idstring>& list)
{
    for (const auto& v : list) {
        if (v.isValid()) {
            xml.writeTag(tagName);
            xml.writeTagAttribute(u8"name", v);
            xml.writeCloseTag();
        }
    }
}

static void writeTempScriptVariablesVector(Xml::XmlWriter& xml, const RoomScripts& roomScripts)
{
    if (roomScripts.tempFlags.empty() || roomScripts.tempWords.empty()) {
        return;
    }

    xml.writeTag(u8"temp-script-variables");
    writeTempScriptVariablesVector(xml, u8"flag"s, roomScripts.tempFlags);
    writeTempScriptVariablesVector(xml, u8"word"s, roomScripts.tempWords);
    xml.writeCloseTag();
}

class ScriptNodeWriter {
private:
    Xml::XmlWriter& xml;

public:
    explicit ScriptNodeWriter(Xml::XmlWriter& x)
        : xml(x){};

    void writeStatements(const std::vector<ScriptNode>& statements)
    {
        for (auto& s : statements) {
            std::visit(*this, s);
        }
    }

    void writeConditionTag(const Conditional& c)
    {
        xml.writeTag(u8"condition");
        xml.writeTagAttributeEnum(u8"type", c.type, conditionalTypeMap);
        xml.writeTagAttributeOptional(u8"var", c.variable);
        xml.writeTagAttributeEnum(u8"comp", c.comparison, comparisonTypeMap);
        xml.writeTagAttributeOptional(u8"value", c.value);
        xml.writeCloseTag();
    }

    void operator()(const Statement& s)
    {
        xml.writeTag(u8"statement"s);
        xml.writeTagAttribute(u8"opcode"s, s.opcode);
        for (auto [i, arg] : const_enumerate(s.arguments)) {
            if (!arg.empty()) {
                xml.writeTagAttribute(attributeTagNames.at(i), arg);
            }
        }
        xml.writeCloseTag();
    }

    void operator()(const IfStatement& s)
    {
        xml.writeTag(u8"if"s);
        writeConditionTag(s.condition);
        writeStatements(s.thenStatements);

        xml.writeCloseTag();

        if (!s.elseStatements.empty()) {
            xml.writeTag(u8"else");
            writeStatements(s.elseStatements);
            xml.writeCloseTag();
        }
    }

    void operator()(const WhileStatement& s)
    {
        xml.writeTag(u8"while"s);
        writeConditionTag(s.condition);
        writeStatements(s.statements);

        xml.writeCloseTag();
    }

    void operator()(const Comment& c)
    {
        xml.writeTag(u8"comment");
        xml.writeTagAttribute(u8"c", c.text);
        xml.writeCloseTag();
    }
};

void writeRoomScripts(Xml::XmlWriter& xml, const RoomScripts& roomScripts)
{
    ScriptNodeWriter snWriter(xml);

    auto writeScript = [&](const Script& script, const idstring& name) {
        xml.writeTag(u8"script"s);
        xml.writeTagAttribute(u8"name"s, name);
        snWriter.writeStatements(script.statements);
        xml.writeCloseTag();
    };

    writeTempScriptVariablesVector(xml, roomScripts);

    writeScript(roomScripts.startupScript, STARTUP_SCRIPT_NAME);

    for (const auto& script : roomScripts.scripts) {
        if (script.name != STARTUP_SCRIPT_NAME) {
            writeScript(script, script.name);
        }
        else {
            writeScript(script, idstring{});
        }
    }
}

}
