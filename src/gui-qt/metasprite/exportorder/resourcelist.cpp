/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcelist.h"
#include "resourceitem.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/project.h"
#include "models/metasprite/frameset-exportorder.h"
#include "models/project/project.h"

#include <QFileInfo>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::ExportOrder;

ResourceList::ResourceList(Project* project)
    : AbstractResourceList(project, ResourceTypeIndex::MS_EXPORT_ORDER)
{
}

UnTech::ExternalFileList<UnTech::MetaSprite::FrameSetExportOrder>& ResourceList::exportOrders() const
{
    return project()->projectFile()->frameSetExportOrders;
}

const QString ResourceList::resourceTypeNameSingle() const
{
    return tr("FrameSet Export Order");
}

const QString ResourceList::resourceTypeNamePlural() const
{
    return tr("FrameSet Export Orders");
}

size_t ResourceList::nItems() const
{
    return exportOrders().size();
}

AbstractResourceItem* ResourceList::buildResourceItem(size_t index)
{
    return new ResourceItem(this, index);
}

const QVector<AbstractResourceList::AddResourceSettings>& ResourceList::addResourceSettings() const
{
    const static QVector<AbstractResourceList::AddResourceSettings> settings = {
        { tr("Add FrameSet Export Order"),
          QString::fromUtf8("MetaSprite FrameSet Export Order (*.utfseo)"),
          QString::fromUtf8("utfseo"),
          true },
    };

    return settings;
}

void ResourceList::do_addResource(int settingIndex, const std::string& fn)
{
    using namespace UnTech::MetaSprite;

    const std::filesystem::path filename(fn);

    Q_ASSERT(settingIndex == 0);

    QFileInfo fi(QString::fromStdString(fn));
    if (!fi.exists()) {
        QString name = fi.baseName();
        IdstringValidator().fixup(name);

        FrameSetExportOrder exportOrder;
        exportOrder.name = name.toStdString();
        saveFrameSetExportOrder(exportOrder, fn);
    }

    auto& exportOrders = this->exportOrders();

    exportOrders.insert_back(fn);
}

void ResourceList::do_removeResource(unsigned index)
{
    auto& exportOrders = this->exportOrders();

    Q_ASSERT(index < exportOrders.size());
    exportOrders.remove(index);
}
