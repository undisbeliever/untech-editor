/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityromentriesresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "gui-qt/project.h"
#include "models/entity/entityromdata.h"
#include "models/project/project.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Entity {

namespace EN = UnTech::Entity;

class EntityRomEntriesList : public QObject {
    Q_OBJECT

public:
    using DataT = EN::EntityRomEntry;
    using ListT = NamedList<DataT>;
    using index_type = ListT::size_type;

    constexpr static index_type max_size = 255;

    EntityRomEntriesList(EntityRomEntriesResourceItem* resourceItem, bool entityList);
    ~EntityRomEntriesList() = default;

    static QString typeName() { return tr("Entity ROM Struct"); }

    EntityRomEntriesResourceItem* resourceItem() const { return _resourceItem; }

    index_type selectedIndex() const { return _selectedIndex; }
    void setSelectedIndex(index_type index);
    void unselectItem() { setSelectedIndex(INT_MAX); }

    bool isSelectedIndexValid() const;

    const DataT* selectedEntry() const;

    void editSelected_setName(const idstring& name);
    void editSelected_setFunctionTable(const idstring& functionTable);
    void editSelected_setComment(const std::string& comment);

    bool editSelected_setInitialListId(const idstring& initialListId);
    bool editSelected_setFrameSetId(const idstring& frameSetId);
    bool editSelected_setDisplayFrame(const idstring& displayFrame);
    bool editSelected_setDefaultPalette(unsigned defaultPalette);
    bool editSelected_setField(const QString& field, const std::string& value);

    const ListT* list() const
    {
        const auto* projectFile = _resourceItem->project()->projectFile();
        Q_ASSERT(projectFile);
        if (_entityList) {
            return &projectFile->entityRomData.entities;
        }
        else {
            return &projectFile->entityRomData.projectiles;
        }
    }

protected:
    friend class Accessor::NamedListUndoHelper<EntityRomEntriesList>;
    ListT* getList()
    {
        auto* projectFile = _resourceItem->project()->projectFile();
        Q_ASSERT(projectFile);
        if (_entityList) {
            return &projectFile->entityRomData.entities;
        }
        else {
            return &projectFile->entityRomData.projectiles;
        }
    }

signals:
    void nameChanged(index_type index);
    void implementsChanged(index_type);
    void commentChanged(index_type);

    void dataChanged(index_type index);
    void listChanged();

    void listAboutToChange();
    void itemAdded(index_type index);
    void itemAboutToBeRemoved(index_type index);
    void itemMoved(index_type from, index_type to);

    void selectedIndexChanged();

private:
    EntityRomEntriesResourceItem* const _resourceItem;
    const bool _entityList;

    index_type _selectedIndex;
};
}
}
}
