/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "metaspriteproject.h"
#include "gui-qt/abstractresourcelist.h"
#include <QObject>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {

class FrameSetResourceList : public AbstractResourceList {
    Q_OBJECT

    enum SettingIndexes {
        METASPRITE_ITEM,
        SPRITEIMPORTER_ITEM
    };

public:
    FrameSetResourceList(MetaSpriteProject* project, ResourceTypeIndex typeIndex);
    ~FrameSetResourceList() = default;

    MetaSpriteProject* project() const { return static_cast<MetaSpriteProject*>(_project); }

    virtual const QString resourceTypeNameSingle() const final;
    virtual const QString resourceTypeNamePlural() const final;

    virtual const QList<AddResourceSettings>& addResourceSettings() const final;

protected:
    virtual size_t nItems() const final;
    virtual AbstractResourceItem* buildResourceItem(size_t index) final;

    virtual void do_addResource(int settingIndex, const std::string& filename) final;
    virtual void do_removeResource(unsigned index) final;
};
}
}
}
