/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "scripting-serializer.h"

namespace UnTech::Scripting {

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

}
