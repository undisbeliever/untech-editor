/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "exportorderresourcelist.h"
#include "exportorderresourceitem.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/project.h"
#include "models/metasprite/frameset-exportorder.h"
#include "models/project/project.h"

#include <QFileInfo>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::ExportOrder;

ExportOrderResourceList::ExportOrderResourceList(Project* project)
    : AbstractResourceList(project, ResourceTypeIndex::MS_EXPORT_ORDER)
{
}

UnTech::ExternalFileList<UnTech::MetaSprite::FrameSetExportOrder>& ExportOrderResourceList::exportOrders() const
{
    return project()->projectFile()->frameSetExportOrders;
}

const QString ExportOrderResourceList::resourceTypeNameSingle() const
{
    return tr("FrameSet Export Order");
}

const QString ExportOrderResourceList::resourceTypeNamePlural() const
{
    return tr("FrameSet Export Orders");
}

size_t ExportOrderResourceList::nItems() const
{
    return exportOrders().size();
}

AbstractResourceItem* ExportOrderResourceList::buildResourceItem(size_t index)
{
    return new ExportOrderResourceItem(this, index);
}

const QList<AbstractResourceList::AddResourceSettings>& ExportOrderResourceList::addResourceSettings() const
{
    const static QList<AbstractResourceList::AddResourceSettings> settings = {
        { tr("Add FrameSet Export Order"),
          QString::fromUtf8("MetaSprite FrameSet Export Order (*.utfseo)"),
          QString::fromUtf8("utfseo"),
          true },
    };

    return settings;
}

void ExportOrderResourceList::do_addResource(int settingIndex, const std::string& filename)
{
    using namespace UnTech::MetaSprite;

    Q_ASSERT(settingIndex == 0);

    QFileInfo fi(QString::fromStdString(filename));
    if (!fi.exists()) {
        QString name = fi.baseName();
        IdstringValidator().fixup(name);

        FrameSetExportOrder exportOrder;
        exportOrder.name = name.toStdString();
        saveFrameSetExportOrder(exportOrder, filename);
    }

    auto& exportOrders = this->exportOrders();

    exportOrders.insert_back(filename);
}

void ExportOrderResourceList::do_removeResource(unsigned index)
{
    auto& exportOrders = this->exportOrders();

    Q_ASSERT(index < exportOrders.size());
    exportOrders.remove(index);
}
