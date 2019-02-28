/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityfunctiontablesmanager.h"
#include "accessors.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "gui-qt/metasprite/exportorder/exportorderresourcelist.h"
#include "gui-qt/metasprite/framesetresourcelist.h"
#include "gui-qt/project.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Entity;

const QStringList EntityFunctionTablesManager::PARAMETER_TYPE_STRINGS = {
    QStringLiteral("Unused"),
    QStringLiteral("Word"),
    QStringLiteral("Signed Word"),
};

EntityFunctionTablesManager::EntityFunctionTablesManager(QObject* parent)
    : PropertyTableManager(parent)
    , _ftList(nullptr)
{
    setItemsMovable(true);

    addProperty(tr("Name"), NAME, PropertyType::IDSTRING);
    addProperty(tr("Entity ROM Struct"), ENTITY_STRUCT, PropertyType::COMBO);
    addProperty(tr("FrameSet Export Order"), EXPORT_ORDER, PropertyType::COMBO);
    addProperty(tr("Parameter Type"), PARAMETER_TYPE, PropertyType::COMBO, PARAMETER_TYPE_STRINGS, qVariantRange(PARAMETER_TYPE_STRINGS.size()));
    addProperty(tr("Comment"), COMMENT, PropertyType::STRING);
}

void EntityFunctionTablesManager::setFunctionTableList(EntityFunctionTableList* ftList)
{
    if (_ftList == ftList) {
        return;
    }

    if (_ftList) {
        _ftList->disconnect(this);
    }
    _ftList = ftList;

    emit dataReset();
    setEnabled(ftList != nullptr);

    if (ftList) {
        connect(ftList, &EntityFunctionTableList::dataChanged,
                this, &EntityFunctionTablesManager::itemChanged);
        connect(ftList, &EntityFunctionTableList::itemAdded,
                this, &EntityFunctionTablesManager::itemAdded);
        connect(ftList, &EntityFunctionTableList::itemAboutToBeRemoved,
                this, &EntityFunctionTablesManager::itemRemoved);
        connect(ftList, &EntityFunctionTableList::itemMoved,
                this, &EntityFunctionTablesManager::itemMoved);

        // Closes all open editors
        connect(ftList, &EntityFunctionTableList::listAboutToChange,
                this, &EntityFunctionTablesManager::listAboutToChange);
    }
}

void EntityFunctionTablesManager::updateParameters(int, int id, QVariant& param1, QVariant&) const
{
    if (_ftList == nullptr) {
        return;
    }

    auto* project = _ftList->resourceItem()->project();
    Q_ASSERT(project);

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::ENTITY_STRUCT:
        param1 = convertNameListWithBlank(project->projectFile()->entityRomData.structs);
        break;

    case PropertyId::EXPORT_ORDER:
        param1 = project->frameSetExportOrderResourceList()->itemNames();
        break;

    case PropertyId::NAME:
    case PropertyId::PARAMETER_TYPE:
    case PropertyId::COMMENT:
        break;
    }
}

int EntityFunctionTablesManager::rowCount() const
{
    if (_ftList == nullptr) {
        return 0;
    }
    return _ftList->size();
}

QVariant EntityFunctionTablesManager::data(int index, int id) const
{
    if (_ftList == nullptr
        || index < 0
        || (unsigned)index >= _ftList->size()) {

        return QVariant();
    }

    const auto& ft = _ftList->list()->at(index);

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return QString::fromStdString(ft.name);

    case PropertyId::ENTITY_STRUCT:
        return QString::fromStdString(ft.entityStruct);

    case PropertyId::EXPORT_ORDER:
        return QString::fromStdString(ft.exportOrder);

    case PropertyId::PARAMETER_TYPE:
        return int(ft.parameterType);

    case PropertyId::COMMENT:
        return QString::fromStdString(ft.comment);
    }

    return QVariant();
}

bool EntityFunctionTablesManager::setData(int index, int id, const QVariant& value)
{
    if (_ftList == nullptr
        || index < 0
        || (unsigned)index >= _ftList->size()) {

        return false;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return _ftList->edit_setName(index, value.toString().toStdString());

    case PropertyId::ENTITY_STRUCT:
        return _ftList->edit_setEntityStruct(index, value.toString().toStdString());

    case PropertyId::EXPORT_ORDER:
        return _ftList->edit_setExportOrder(index, value.toString().toStdString());

    case PropertyId::PARAMETER_TYPE:
        return _ftList->edit_setParameterType(index, static_cast<EN::ParameterType>(value.toInt()));

    case PropertyId::COMMENT:
        return _ftList->edit_setComment(index, value.toString().toStdString());
    }

    return false;
}

bool EntityFunctionTablesManager::canInsertItem()
{
    return _ftList
           && _ftList->size() < _ftList->maxSize();
}

bool EntityFunctionTablesManager::canCloneItem(int index)
{
    return _ftList
           && index >= 0
           && unsigned(index) < _ftList->size()
           && _ftList->size() < _ftList->maxSize();
}

bool EntityFunctionTablesManager::insertItem(int index)
{
    if (_ftList == nullptr) {
        return false;
    }
    return _ftList->addItem(index);
}

bool EntityFunctionTablesManager::cloneItem(int index)
{
    if (_ftList == nullptr) {
        return false;
    }
    return _ftList->cloneItem(index);
}

bool EntityFunctionTablesManager::removeItem(int index)
{
    if (_ftList == nullptr) {
        return false;
    }
    return _ftList->removeItem(index);
}

bool EntityFunctionTablesManager::moveItem(int from, int to)
{
    if (_ftList == nullptr) {
        return false;
    }
    return _ftList->moveItem(from, to);
}
