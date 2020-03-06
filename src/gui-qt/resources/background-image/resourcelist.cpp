/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcelist.h"
#include "resourceitem.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/project.h"
#include "models/common/imagecache.h"
#include "models/project/project.h"

#include <QFileInfo>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources::BackgroundImage;

ResourceList::ResourceList(Project* project)
    : AbstractResourceList(project, ResourceTypeIndex::PALETTE)
{
}

const QString ResourceList::resourceTypeNameSingle() const
{
    return tr("Background Image");
}

const QString ResourceList::resourceTypeNamePlural() const
{
    return tr("Background Images");
}

UnTech::NamedList<RES::BackgroundImageInput>& ResourceList::backgroundImages() const
{
    return project()->projectFile()->backgroundImages;
}

size_t ResourceList::nItems() const
{
    return backgroundImages().size();
}

ResourceItem* ResourceList::buildResourceItem(size_t index)
{
    return new ResourceItem(this, index);
}

const QList<AbstractResourceList::AddResourceSettings>& ResourceList::addResourceSettings() const
{
    const static QList<AbstractResourceList::AddResourceSettings> filters = {
        { tr("Add Background Image"),
          QString::fromUtf8("PNG Image (*.png)"),
          QString::fromUtf8("png"),
          .canCreateFile = false }
    };

    return filters;
}

ResourceItem* ResourceList::findResource(const QString& name) const
{
    return qobject_cast<ResourceItem*>(AbstractResourceList::findResource(name));
}

void ResourceList::do_addResource(int settingIndex, const std::string& filename)
{
    Q_ASSERT(settingIndex == 0);

    auto& palettes = this->backgroundImages();

    palettes.insert_back();
    RES::BackgroundImageInput& bi = palettes.back();

    QFileInfo fi = QString::fromStdString(filename);
    QString name = fi.baseName();
    IdstringValidator().fixup(name);

    bi.name = name.toStdString();
    bi.imageFilename = filename;
}

void ResourceList::do_removeResource(unsigned index)
{
    auto& backgroundImages = this->backgroundImages();

    Q_ASSERT(index < backgroundImages.size());
    backgroundImages.remove(index);
}
