/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "metatile-tileset.h"
#include "models/common/xml/xmlreader.h"
#include "models/resources/error-list.h"

namespace UnTech {
namespace MetaTiles {

// returns nullptr on error
std::unique_ptr<MetaTileTilesetInput> loadMetaTileTilesetInput(const std::string& filename, Resources::ErrorList& err);

// raises an exception on error
std::unique_ptr<MetaTileTilesetInput> loadMetaTileTilesetInput(const std::string& filename);

void readEngineSettings(EngineSettings& settings, const Xml::XmlTag* tag);
}
}
