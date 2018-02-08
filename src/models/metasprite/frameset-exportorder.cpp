/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "frameset-exportorder.h"
#include "models/common/humantypename.h"

using namespace UnTech;
using namespace UnTech::MetaSprite;

template <>
const std::string HumanTypeName<FrameSetExportOrder>::value = "FrameSet Export Order";

template <>
const std::string HumanTypeName<FrameSetExportOrder::ExportName>::value = "Export Name";
