/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QMenu>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {

extern const QStringList TILESET_TYPE_STRINGS;
extern const QVariantList TILESET_TYPE_VALUES;

extern const QStringList EH_SHORT_STRING_VALUES;
extern const QStringList EH_LONG_STRING_VALUES;

void populateEntityHitboxTypeMenu(QMenu* menu);
}
}
}
