/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "metatile-tileset.h"
#include "models/common/xml/xmlreader.h"

namespace UnTech {
namespace MetaTiles {

std::unique_ptr<MetaTileTilesetInput> loadMetaTileTilesetInput(const std::string& filename);

void readEngineSettings(EngineSettings& settings, const Xml::XmlTag* tag);
}
}
