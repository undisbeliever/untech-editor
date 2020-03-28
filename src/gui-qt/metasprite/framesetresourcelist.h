/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourcelist.h"
#include "models/metasprite/framesetfile.h"
#include <QObject>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractMsResourceItem;

class FrameSetResourceList : public AbstractResourceList {
    Q_OBJECT

    enum SettingIndexes {
        METASPRITE_ITEM,
        SPRITEIMPORTER_ITEM
    };

public:
    FrameSetResourceList(Project* project);
    ~FrameSetResourceList() = default;

    virtual const QString resourceTypeNameSingle() const final;
    virtual const QString resourceTypeNamePlural() const final;

    virtual const QVector<AddResourceSettings>& addResourceSettings() const final;

    AbstractMsResourceItem* findResource(const QString& name) const;

protected:
    virtual size_t nItems() const final;
    virtual AbstractResourceItem* buildResourceItem(size_t index) final;

    virtual void do_addResource(int settingIndex, const std::string& filename) final;
    virtual void do_removeResource(unsigned index) final;

    // Will always return the same instance
    friend class AbstractMsResourceItem;
    friend class NullFrameSetResourceItem;
    std::vector<UnTech::MetaSprite::FrameSetFile>& frameSetFiles() const;
};
}
}
}
