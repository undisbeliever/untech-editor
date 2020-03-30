/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "models/common/externalfilelist.h"
#include "models/common/optional.h"
#include "models/rooms/rooms.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Rooms {
class ResourceList;
class MapGrid;

namespace RM = UnTech::Rooms;

class ResourceItem final : public AbstractExternalResourceItem {
    Q_OBJECT

    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

public:
    using DataT = RM::RoomInput;

public:
    ResourceItem(ResourceList* parent, size_t index);
    ~ResourceItem() = default;

    static QString typeName() { return tr("Room"); }

    inline optional<const RM::RoomInput&> roomInput() const
    {
        if (auto* ri = _rooms.at(index())) {
            return *ri;
        }
        else {
            return {};
        }
    }
    inline auto& roomInputItem()
    {
        return _rooms.item(index());
    }

    MapGrid* mapGrid() const { return _mapGrid; }

    bool edit_setName(const idstring& name);
    bool edit_setScene(const idstring& name);

private:
    friend class Accessor::ResourceItemUndoHelper<ResourceItem>;
    friend class MapGrid;
    const DataT* data() const { return _rooms.at(index()); }
    DataT* dataEditable() { return _rooms.at(index()); }

    void updateExternalFiles();
    void updateDependencies();

protected:
    virtual void saveResourceData(const std::filesystem::path& filename) const final;
    virtual bool loadResourceData(ErrorList& err) final;
    virtual bool compileResource(ErrorList& err) final;

signals:
    void sceneChanged();

private:
    ExternalFileList<DataT>& _rooms;

    MapGrid* const _mapGrid;
};
}
}
}
