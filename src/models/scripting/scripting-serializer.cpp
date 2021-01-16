/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "scripting-serializer.h"
#include "models/common/enummap.h"

namespace UnTech::Scripting {

using namespace std::string_literals;

static const EnumMap<ArgumentType> argumentTypeEnumMap = {
    { "unused", ArgumentType::Unused },
    { "flag", ArgumentType::Flag },
    { "word", ArgumentType::Word },
    { "immediate-u16", ArgumentType::ImmediateU16 },
};

void readGameState(GameState& gameState, Xml::XmlReader& xml, const Xml::XmlTag* tag)
{
    assert(tag->name == "game-state");
    assert(gameState.flags.empty());
    assert(gameState.words.empty());

    gameState.startingRoom = tag->getAttributeOptionalId("starting-room");
    gameState.startingEntrance = tag->getAttributeOptionalId("starting-entrance");
    gameState.startingPlayer = tag->getAttributeOptionalId("starting-player");

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "flag") {
            const unsigned index = childTag->getAttributeUnsigned("index", 0, GameState::MAX_FLAGS * 2);

            if (index >= gameState.flags.size()) {
                gameState.flags.resize(index + 1);
            }
            auto& f = gameState.flags.at(index);

            f.name = childTag->getAttributeOptionalId("name");
            f.room = childTag->getAttributeOptionalId("room");
        }
        else if (childTag->name == "word") {
            const unsigned index = childTag->getAttributeUnsigned("index", 0, GameState::MAX_FLAGS * 2);

            if (index >= gameState.words.size()) {
                gameState.words.resize(index + 1);
            }
            auto& w = gameState.words.at(index);

            w.name = childTag->getAttributeOptionalId("name");
            w.room = childTag->getAttributeOptionalId("room");

            if (childTag->hasAttribute("value")) {
                w.initialValue = childTag->getAttributeUint16("value");
            }
        }
        else {
            throw Xml::unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }
}

void writeGameState(Xml::XmlWriter& xml, const GameState& gameState)
{
    xml.writeTag("game-state");

    xml.writeTagAttribute("starting-room", gameState.startingRoom);
    xml.writeTagAttribute("starting-entrance", gameState.startingEntrance);
    xml.writeTagAttribute("starting-player", gameState.startingPlayer);

    for (unsigned i = 0; i < gameState.flags.size(); i++) {
        auto& f = gameState.flags.at(i);
        if (f.name.isValid()) {
            xml.writeTag("flag");
            xml.writeTagAttribute("index", i);
            xml.writeTagAttributeOptional("name", f.name);
            xml.writeTagAttributeOptional("room", f.room);
            xml.writeCloseTag();
        }
    }

    for (unsigned i = 0; i < gameState.words.size(); i++) {
        auto& w = gameState.words.at(i);
        if (w.name.isValid()) {
            xml.writeTag("word");
            xml.writeTagAttribute("index", i);
            xml.writeTagAttributeOptional("name", w.name);
            xml.writeTagAttributeOptional("room", w.room);
            xml.writeTagAttribute("value", w.initialValue);
            xml.writeCloseTag();
        }
    }

    xml.writeCloseTag();
}

void readBytecode(BytecodeInput& bytecode, Xml::XmlReader& xml, const Xml::XmlTag* tag)
{
    assert(tag->name == "bytecode");
    assert(bytecode.instructions.empty());

    while (auto childTag = xml.parseTag()) {
        if (childTag->name == "instruction") {
            Instruction& inst = bytecode.instructions.emplace_back();

            inst.name = childTag->getAttributeOptionalId("name");

            auto readArg = [&](const std::string& name) {
                return childTag->getAttributeOptionalEnum(name, argumentTypeEnumMap, ArgumentType::Unused);
            };
            inst.arguments.at(0) = readArg("arg1");
            inst.arguments.at(1) = readArg("arg2");
            assert(inst.arguments.size() == 2);
        }
        else {
            throw Xml::unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }
}

void writeBytecode(Xml::XmlWriter& xml, const BytecodeInput& bytecode)
{
    using namespace std::string_literals;

    xml.writeTag("bytecode");

    for (const auto& inst : bytecode.instructions) {
        xml.writeTag("instruction");
        xml.writeTagAttributeOptional("name", inst.name);

        auto writeArg = [&](const std::string& name, ArgumentType arg) {
            if (arg != ArgumentType::Unused) {
                xml.writeTagAttributeEnum(name, arg, argumentTypeEnumMap);
            }
        };
        writeArg("arg1"s, inst.arguments.at(0));
        writeArg("arg2"s, inst.arguments.at(1));
        assert(inst.arguments.size() == 2);

        xml.writeCloseTag();
    }

    xml.writeCloseTag();
}

static const std::array<std::string, 2> attributeTagNames = {
    "arg1"s,
    "arg2"s,
};

static void readScriptNodes(std::vector<ScriptNode>& nodes, Xml::XmlReader& xml)
{
    while (auto tag = xml.parseTag()) {
        if (tag->name == "statement"s) {
            auto& s = std::get<Statement>(nodes.emplace_back(Statement{}));

            s.opcode = tag->getAttributeOptionalId("opcode"s);
            for (unsigned i = 0; i < s.arguments.size(); i++) {
                s.arguments.at(i) = tag->getAttributeOrEmpty(attributeTagNames.at(i));
            }
        }
        else {
            throw Xml::unknown_tag_error(*tag);
        }

        xml.parseCloseTag();
    }
}

void readScript(NamedList<Script>& scripts, Xml::XmlReader& xml, const Xml::XmlTag* tag)
{
    assert(tag->name == "script"s);

    scripts.emplace_back();
    auto& script = scripts.back();

    script.name = tag->getAttributeOptionalId("name"s);

    readScriptNodes(script.statements, xml);
}

class ScriptNodeWriter {
private:
    Xml::XmlWriter& xml;

public:
    ScriptNodeWriter(Xml::XmlWriter& x)
        : xml(x){};

    void operator()(const Statement& s)
    {
        xml.writeTag("statement"s);
        xml.writeTagAttribute("opcode"s, s.opcode);
        for (unsigned i = 0; i < s.arguments.size(); i++) {
            auto& arg = s.arguments.at(i);
            if (!arg.empty()) {
                xml.writeTagAttribute(attributeTagNames.at(i), s.arguments.at(i));
            }
        }
        xml.writeCloseTag();
    }
};

void writeScripts(Xml::XmlWriter& xml, const NamedList<Script>& scripts)
{
    ScriptNodeWriter snWriter(xml);

    for (auto& script : scripts) {
        xml.writeTag("script"s);
        xml.writeTagAttribute("name"s, script.name);

        for (auto& sn : script.statements) {
            std::visit(snWriter, sn);
        }

        xml.writeCloseTag();
    }
}

}
