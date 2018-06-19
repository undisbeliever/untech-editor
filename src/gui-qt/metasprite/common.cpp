/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "common.h"
#include "models/metasprite/entityhitboxtype.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite;

void MetaSprite::populateEntityHitboxTypeMenu(QMenu* menu)
{
    using EHT = UnTech::MetaSprite::EntityHitboxType;

    Q_ASSERT(menu->actions().size() == 0);
    Q_ASSERT(EHT::SHORT_STRING_VALUES.size() == EHT::LONG_STRING_VALUES.size());

    menu->setToolTipsVisible(true);

    for (unsigned i = 0; i < EHT::SHORT_STRING_VALUES.size(); i++) {
        QString title = QString::fromStdString(EHT::SHORT_STRING_VALUES.at(i));
        QString tooltip = QString::fromStdString(EHT::LONG_STRING_VALUES.at(i));

        QAction* a = menu->addAction(title);
        a->setData(int(i));
        a->setToolTip(tooltip);
    }
}
