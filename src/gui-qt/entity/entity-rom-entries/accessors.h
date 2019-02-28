/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityromentriesresourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/accessor/accessor.h"
#include "gui-qt/project.h"
#include "models/entity/entityromdata.h"
#include "models/project/project.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Entity {

namespace EN = UnTech::Entity;

class EntityRomEntriesList : public Accessor::NamedListAccessor<EN::EntityRomEntry, EntityRomEntriesResourceItem> {
    Q_OBJECT

    friend class Accessor::NamedListUndoHelper<EntityRomEntriesList>;

private:
    const bool _entityList;

public:
    EntityRomEntriesList(EntityRomEntriesResourceItem* resourceItem, bool entityList);
    ~EntityRomEntriesList() = default;

    virtual QString typeName() const final;

    bool isEntityList() const { return _entityList; }

    void editSelected_setFunctionTable(const idstring& functionTable);
    void editSelected_setComment(const std::string& comment);

    bool editSelected_setInitialListId(const idstring& initialListId);
    bool editSelected_setFrameSetId(const idstring& frameSetId);
    bool editSelected_setDisplayFrame(const idstring& displayFrame);
    bool editSelected_setDefaultPalette(unsigned defaultPalette);
    bool editSelected_setField(const QString& field, const std::string& value);

signals:
    void implementsChanged(index_type);
    void commentChanged(index_type);
};

}
}
}
