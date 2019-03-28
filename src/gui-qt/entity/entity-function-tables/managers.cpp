/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "gui-qt/metasprite/exportorder/exportorderresourcelist.h"
#include "gui-qt/metasprite/framesetresourcelist.h"
#include "gui-qt/project.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Entity::EntityFunctionTables;

const QStringList EntityFunctionTablesManager::PARAMETER_TYPE_STRINGS = {
    QStringLiteral("Unused"),
    QStringLiteral("Word"),
    QStringLiteral("Signed Word"),
};

EntityFunctionTablesManager::EntityFunctionTablesManager(QObject* parent)
    : ListAccessorTableManager(parent)
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
    setAccessor(ftList);
}

inline EntityFunctionTableList* EntityFunctionTablesManager::accessor() const
{
    return static_cast<EntityFunctionTableList*>(ListAccessorTableManager::accessor());
}

void EntityFunctionTablesManager::updateParameters(int, int id, QVariant& param1, QVariant&) const
{
    auto* ftList = accessor();
    if (ftList == nullptr) {
        return;
    }

    auto* project = ftList->resourceItem()->project();
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

QVariant EntityFunctionTablesManager::data(int index, int id) const
{
    auto* ftList = accessor();
    if (ftList == nullptr
        || index < 0
        || (unsigned)index >= ftList->size()) {

        return QVariant();
    }

    const auto& ft = ftList->list()->at(index);

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
    auto* ftList = accessor();
    if (ftList == nullptr
        || index < 0
        || (unsigned)index >= ftList->size()) {

        return false;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return ftList->edit_setName(index, value.toString().toStdString());

    case PropertyId::ENTITY_STRUCT:
        return ftList->edit_setEntityStruct(index, value.toString().toStdString());

    case PropertyId::EXPORT_ORDER:
        return ftList->edit_setExportOrder(index, value.toString().toStdString());

    case PropertyId::PARAMETER_TYPE:
        return ftList->edit_setParameterType(index, static_cast<EN::ParameterType>(value.toInt()));

    case PropertyId::COMMENT:
        return ftList->edit_setComment(index, value.toString().toStdString());
    }

    return false;
}
