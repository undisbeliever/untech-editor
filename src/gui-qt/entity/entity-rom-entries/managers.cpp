/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"
#include "entityromentriesresourceitem.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "gui-qt/metasprite/framesetresourcelist.h"
#include "gui-qt/project.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Entity;

EntityRomEntryManager::EntityRomEntryManager(QObject* parent)
    : PropertyListManager(parent)
    , _structFields()
    , _entryFields()
    , _item(nullptr)
{
}

void EntityRomEntryManager::setResourceItem(EntityRomEntriesResourceItem* item)
{
    if (_item == item) {
        return;
    }

    if (_item) {
        _item->disconnect(this);
        _item->entriesList()->disconnect(this);
    }
    _item = item;

    if (item) {
        _structFields.setEntityRomData(&item->project()->projectFile()->entityRomData);

        connect(item->entriesList(), &EntityRomEntriesList::selectedIndexChanged,
                this, &EntityRomEntryManager::onSelectedEntryChanged);
        connect(item->entriesList(), &EntityRomEntriesList::dataChanged,
                this, &EntityRomEntryManager::onEntryDataChanged);
        connect(item->entriesList(), &EntityRomEntriesList::implementsChanged,
                this, &EntityRomEntryManager::onImplementsChanged);
    }

    onSelectedEntryChanged();
}

void EntityRomEntryManager::onSelectedEntryChanged()
{
    rebuildStructFields();
    rebuildEntryFields();

    setEnabled(_item && _item->entriesList()->isSelectedIndexValid());
}

void EntityRomEntryManager::onImplementsChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->entriesList()->selectedIndex()) {
        rebuildStructFields();
    }
}

void EntityRomEntryManager::onEntryDataChanged(size_t index)
{
    Q_ASSERT(_item);
    if (index == _item->entriesList()->selectedIndex()) {
        rebuildEntryFields();
    }
}

void EntityRomEntryManager::rebuildStructFields()
{
    _structFields.clear();

    if (_item) {
        if (auto* e = _item->entriesList()->selectedItem()) {
            const auto& entityRomData = _item->project()->projectFile()->entityRomData;
            if (auto ft = entityRomData.functionTables.find(e->functionTable)) {
                if (auto s = entityRomData.structs.find(ft->entityStruct)) {
                    _structFields.rebuildDataForStruct(&*s);
                }
            }
        }
    }

    QVector<Property> properties;
    properties.reserve(_structFields.size() + FIXED_FIELDS_SIZE);

    properties.append(Property(QStringLiteral("initialProjectileId"), INITIAL_PROJECTILE_ID, PropertyType::COMBO));
    properties.append(Property(QStringLiteral("initialListId"), INTITAL_LIST_ID, PropertyType::COMBO));
    properties.append(Property(QStringLiteral("frameSetId"), FRAME_SET_ID, PropertyType::COMBO));
    properties.append(Property(QStringLiteral("defaultPalette"), DEFAULT_PALETTE, PropertyType::UNSIGNED, 0, 255));
    properties.append(Property(QStringLiteral("displayFrame"), DISPLAY_FRAME, PropertyType::COMBO));

    for (int i = 0; i < _structFields.size(); i++) {
        const auto& field = _structFields.at(i);
        properties.append(Property(field.name, FIXED_FIELDS_SIZE + i, PropertyType::STRING));
    }
    setPropertyList(properties);
}

void EntityRomEntryManager::rebuildEntryFields()
{
    _entryFields.clear();

    if (_item) {
        if (auto* e = _item->entriesList()->selectedItem()) {
            for (const auto& it : e->fields) {
                if (it.second.empty() == false) {
                    _entryFields.insert(QString::fromStdString(it.first),
                                        QString::fromStdString(it.second));
                }
            }
        }
    }

    emit dataChanged();
}

void EntityRomEntryManager::updateParameters(int id, QVariant& param1, QVariant& param2) const
{
    if (_item == nullptr
        || id < 0 || id >= FIXED_FIELDS_SIZE) {
        return;
    }

    const auto* frameSetResourceList = _item->project()->frameSetResourceList();
    auto getFrameSet = [&]() -> const MetaSprite::AbstractMsDocument* {
        if (auto* e = _item->entriesList()->selectedItem()) {
            return qobject_cast<const MetaSprite::AbstractMsDocument*>(
                frameSetResourceList->findResource(QString::fromStdString(e->frameSetId)));
        }
        return nullptr;
    };

    switch (static_cast<FixedFieldIds>(id)) {
    case INITIAL_PROJECTILE_ID:
        param1 = convertNameListWithBlank(_item->project()->projectFile()->entityRomData.projectiles);
        break;

    case INTITAL_LIST_ID:
        param1 = convertStringList(_item->project()->projectFile()->entityRomData.listIds);
        break;

    case FRAME_SET_ID:
        // Only show frameSets that match the function table export order
        if (auto* e = _item->entriesList()->selectedItem()) {
            auto* projectFile = _item->project()->projectFile();
            Q_ASSERT(projectFile);
            if (auto ft = projectFile->entityRomData.functionTables.find(e->functionTable)) {
                QStringList qsl;
                qsl.reserve(projectFile->frameSets.size());
                for (auto& fs : projectFile->frameSets) {
                    if (fs.exportOrder() == ft->exportOrder) {
                        qsl.append(QString::fromStdString(fs.name()));
                    }
                }
                param1 = qsl;
            }
        }
        break;

    case DISPLAY_FRAME:
        if (auto* fs = getFrameSet()) {
            param1 = fs->frameNames();
        }
        break;

    case DEFAULT_PALETTE:
        if (auto* fs = getFrameSet()) {
            param1 = 0;
            param2 = qBound(0U, fs->nPalettes() - 1, 255U);
        }
        break;
    }
}

QVariant EntityRomEntryManager::data(int id) const
{
    if (_item == nullptr
        || id < 0 || id >= propertiesList().size()) {

        return QVariant();
    }

    auto* entry = _item->entriesList()->selectedItem();
    if (entry == nullptr) {
        return QVariant();
    }

    if (id < FIXED_FIELDS_SIZE) {
        switch (static_cast<FixedFieldIds>(id)) {
        case INITIAL_PROJECTILE_ID:
            return QString::fromStdString(entry->initialProjectileId);

        case INTITAL_LIST_ID:
            return QString::fromStdString(entry->initialListId);

        case FRAME_SET_ID:
            return QString::fromStdString(entry->frameSetId);

        case DISPLAY_FRAME:
            return QString::fromStdString(entry->displayFrame);

        case DEFAULT_PALETTE:
            return entry->defaultPalette;
        }
    }
    else if (id >= FIXED_FIELDS_SIZE) {
        int fieldId = id - FIXED_FIELDS_SIZE;

        const auto& f = _structFields.at(fieldId);
        return _entryFields.value(f.name, f.defaultValue);
    }

    return QVariant();
}

bool EntityRomEntryManager::setData(int id, const QVariant& value)
{
    if (_item == nullptr
        || id < 0 || id >= propertiesList().size()) {

        return false;
    }

    if (id < FIXED_FIELDS_SIZE) {
        switch (static_cast<FixedFieldIds>(id)) {
        case INITIAL_PROJECTILE_ID:
            return _item->entriesList()->editSelected_setInitialProjectileId(
                value.toString().toStdString());

        case INTITAL_LIST_ID:
            return _item->entriesList()->editSelected_setInitialListId(
                value.toString().toStdString());

        case FRAME_SET_ID:
            return _item->entriesList()->editSelected_setFrameSetId(
                value.toString().toStdString());

        case DISPLAY_FRAME:
            return _item->entriesList()->editSelected_setDisplayFrame(
                value.toString().toStdString());

        case DEFAULT_PALETTE:
            return _item->entriesList()->editSelected_setDefaultPalette(
                value.toString().toUInt());
        }
    }
    else if (id >= FIXED_FIELDS_SIZE) {
        int fieldId = id - FIXED_FIELDS_SIZE;

        return _item->entriesList()->editSelected_setField(
            _structFields.at(fieldId).name,
            value.toString().toStdString());
    }

    return false;
}
