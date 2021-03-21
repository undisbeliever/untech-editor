/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "interactive-tiles.h"
#include "metatile-tileset.h"
#include "models/common/errorlist.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace MetaTiles {

// returns nullptr on error
std::unique_ptr<MetaTileTilesetInput> loadMetaTileTilesetInput(const std::filesystem::path& filename, ErrorList& err);

// raises an exception on error
std::unique_ptr<MetaTileTilesetInput> loadMetaTileTilesetInput(const std::filesystem::path& filename);

// raises an exception on error
std::unique_ptr<MetaTileTilesetInput> readMetaTileTilesetInput(Xml::XmlReader& xml);

// raises an exception on error
void saveMetaTileTilesetInput(const MetaTileTilesetInput& input, const std::filesystem::path& filename);

// raises an exception on error
void writeMetaTileTilesetInput(Xml::XmlWriter& xml, const MetaTileTilesetInput& input);

grid<uint8_t> readMetaTileGrid(Xml::XmlReader& xml, const Xml::XmlTag& tag);
void writeMetaTileGrid(Xml::XmlWriter& xml, const std::string& tagName, const grid<uint8_t>& mtGrid);

// raises an exception on error
void readInteractiveTiles(Xml::XmlReader& xml, const Xml::XmlTag& tag, InteractiveTiles& interactiveTiles);
void writeInteractiveTiles(Xml::XmlWriter& xml, const InteractiveTiles& interactiveTiles);

}
}
