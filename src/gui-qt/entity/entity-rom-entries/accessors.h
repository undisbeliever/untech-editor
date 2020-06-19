/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/project.h"
#include "models/entity/entityromdata.h"
#include "models/project/project.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Entity {
namespace EntityRomEntries {

namespace EN = UnTech::Entity;

class EntityRomEntriesList : public Accessor::NamedListAccessor<EN::EntityRomEntry, ResourceItem> {
    Q_OBJECT

    using UndoHelper = Accessor::ListAndSelectionUndoHelper<EntityRomEntriesList>;

private:
    const EN::EntityType _entityType;

public:
    EntityRomEntriesList(ResourceItem* resourceItem, const EN::EntityType entityType);
    ~EntityRomEntriesList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    EN::EntityType entityType() const { return _entityType; }

    void editSelected_setFunctionTable(const idstring& functionTable);
    void editSelected_setComment(const std::string& comment);

    bool editSelected_setInitialProjectileId(const idstring& initialProjectileId);
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
}
