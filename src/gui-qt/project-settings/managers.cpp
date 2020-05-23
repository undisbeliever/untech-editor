/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "resourceitem.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/project.h"
#include "models/project/project.h"

using namespace UnTech::GuiQt::ProjectSettings;

namespace RES = UnTech::Resources;
namespace MT = UnTech::MetaTiles;

const QStringList ProjectSettingsPropertyManager::LAYOUT_STRINGS({ QString::fromUtf8("LoROM"),
                                                                   QString::fromUtf8("HiROM") });

ProjectSettingsPropertyManager::ProjectSettingsPropertyManager(QObject* parent)
    : PropertyListManager(parent)
    , _item(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;

    using RS = UnTech::Rooms::RoomSettings;

    addPropertyGroup(tr("Memory Map:"));
    addProperty(tr("Mapping Mode"), MAPPING_MODE, Type::COMBO, LAYOUT_STRINGS, qVariantRange(LAYOUT_STRINGS.size()));
    addProperty(tr("First Bank"), FIRST_BANK, Type::UNSIGNED_HEX, 0, 255);
    addProperty(tr("Number of Banks"), N_BANKS, Type::UNSIGNED, 1, 128);
    addPropertyGroup(tr("Room Settings:"));
    addProperty(tr("Room Data Size"), ROOM_DATA_SIZE, Type::UNSIGNED, RS::MIN_ROOM_DATA_SIZE, RS::MAX_ROOM_DATA_SIZE);
    addPropertyGroup(tr("Entity:"));
    addProperty(tr("EntityListIds"), ENTITY_LIST_IDS, Type::IDSTRING_LIST);
}

void ProjectSettingsPropertyManager::setResourceItem(ResourceItem* item)
{
    if (_item == item) {
        return;
    }

    if (_item) {
        _item->disconnect(this);
    }
    _item = item;

    setEnabled(_item != nullptr);

    if (_item) {
        connect(_item, &ResourceItem::dataChanged,
                this, &ProjectSettingsPropertyManager::dataChanged);
    }

    emit dataChanged();
}

QVariant ProjectSettingsPropertyManager::data(int id) const
{
    if (_item == nullptr) {
        return QVariant();
    }

    const auto* pro = _item->project()->projectFile();
    Q_ASSERT(pro);

    switch ((PropertyId)id) {
    case MAPPING_MODE:
        return int(pro->memoryMap.mode);

    case FIRST_BANK:
        return pro->memoryMap.firstBank;

    case N_BANKS:
        return pro->memoryMap.nBanks;

    case ROOM_DATA_SIZE:
        return pro->roomSettings.roomDataSize;

    case ENTITY_LIST_IDS:
        return convertStringList(pro->entityRomData.listIds);
    }

    return QVariant();
}

bool ProjectSettingsPropertyManager::setData(int id, const QVariant& value)
{
    Q_ASSERT(_item);

    switch ((PropertyId)id) {
    case MAPPING_MODE:
        return _item->editMemoryMap_setMappingMode(PRO::MappingMode(value.toInt()));

    case FIRST_BANK:
        return _item->editBlockSettings_setFirstBank(value.toUInt());

    case N_BANKS:
        return _item->editBlockSettings_setNBanks(value.toUInt());

    case ROOM_DATA_SIZE:
        return _item->editMetaTileSettings_setRoomDataSize(value.toUInt());

    case ENTITY_LIST_IDS:
        return _item->editEntityRomData_setEntityListIds(toIdstringVector(value.toStringList()));
    }

    return false;
}
