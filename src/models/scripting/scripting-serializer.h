/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "game-state.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech::Scripting {

// raises exception on error
// `gameState` must be empty, xml/tag points to a <game-state> tag
void readGameState(GameState& gameState, Xml::XmlReader& xml, const Xml::XmlTag* tag);

void writeGameState(Xml::XmlWriter& xml, const GameState& gameState);

}
