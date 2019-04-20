/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "common.h"
#include "gui-qt/common/helpers.h"
#include "models/metasprite/entityhitboxtype.h"
#include "models/metasprite/tilesettype.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite;

// If you modify this also modify EntityHitboxType::SHORT_STRING_VALUES
// located in src/models/metasprite/entityhitboxtype.cpp
const QStringList MetaSprite::EH_SHORT_STRING_VALUES = {
    "----",
    "---B",
    "--A-",
    "--AB",
    "-S--",
    "-S-B",
    "-SA-",
    "-SAB",
    "W---",
    "W--B",
    "W-A-",
    "W-AB",
    "WS--",
    "WS-B",
    "WSA-",
    "WSAB",
};

// If you modify this also modify EntityHitboxType::LONG_STRING_VALUES
// located in src/models/metasprite/entityhitboxtype.cpp
const QStringList MetaSprite::EH_LONG_STRING_VALUES = {
    QString(),
    "Body",
    "Attack",
    "Attack Body",
    "Shield",
    "Shield Body",
    "Shield Attack",
    "Shield Attack Body",
    "Weak",
    "Weak Body",
    "Weak Attack",
    "Weak Attack Body",
    "Weak Shield",
    "Weak Shield Body",
    "Weak Shield Attack",
    "Weak Shield Attack Body",
};

void MetaSprite::populateEntityHitboxTypeMenu(QMenu* menu)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;

    Q_ASSERT(menu->actions().size() == 0);
    Q_ASSERT(EHT::SHORT_STRING_VALUES.size() == EHT::LONG_STRING_VALUES.size());

    menu->setToolTipsVisible(true);

    for (unsigned i = 0; i < EHT::SHORT_STRING_VALUES.size(); i++) {
        QAction* a = menu->addAction(EH_SHORT_STRING_VALUES.at(i));
        a->setData(int(i));
        a->setToolTip(EH_LONG_STRING_VALUES.at(i));
    }
}
