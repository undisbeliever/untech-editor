/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "bytecode.h"
#include "game-state.h"
#include "script.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech::Scripting {

// raises exception on error
// `gameState` must be empty, xml/tag points to a <game-state> tag
void readGameState(GameState& gameState, Xml::XmlReader& xml, const Xml::XmlTag* tag);

void writeGameState(Xml::XmlWriter& xml, const GameState& gameState);

// raises exception on error
// `gameState` must be empty, xml/tag points to a <bytecode> tag
void readBytecode(Scripting::BytecodeInput& bytecode, Xml::XmlReader& xml, const Xml::XmlTag* tag);

void writeBytecode(Xml::XmlWriter& xml, const Scripting::BytecodeInput& bytecode);

// raises exception on error
// xml/tag points to a <script> tag
void readScript(RoomScripts& roomScripts, Xml::XmlReader& xml, const Xml::XmlTag* tag);

// raises exception on error
// xml/tag points to a <temp-script-variables> tag
void readTempScriptVariables(RoomScripts& roomScripts, Xml::XmlReader& xml, const Xml::XmlTag* tag);

void writeRoomScripts(Xml::XmlWriter& xml, const RoomScripts& roomScripts);

}
