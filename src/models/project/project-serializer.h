/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "project.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"

namespace UnTech {
namespace Project {

std::unique_ptr<ProjectFile> readProjectFile(Xml::XmlReader& xml);
void writeProjectFile(Xml::XmlWriter& xml, const ProjectFile& project);

}
}
