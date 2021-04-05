/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "actionpointfunctions-serializer.h"
#include <cassert>

using namespace UnTech::Xml;

namespace UnTech::MetaSprite {

void readActionPointFunction(const XmlTag& tag, NamedList<ActionPointFunction>& actionPointList)
{
    assert(tag.name == "action-point-function");

    actionPointList.insert_back();
    auto& ap = actionPointList.back();

    ap.name = tag.getAttributeOptionalId("name");
    ap.manuallyInvoked = tag.getAttributeBoolean("manually-invoked");
}

void writeActionPointFunctions(XmlWriter& xml, const NamedList<ActionPointFunction>& actionPointFunctions)
{
    for (const ActionPointFunction& ap : actionPointFunctions) {
        xml.writeTag("action-point-function");
        xml.writeTagAttribute("name", ap.name);
        xml.writeTagAttribute("manually-invoked", ap.manuallyInvoked);
        xml.writeCloseTag();
    }
}

}
