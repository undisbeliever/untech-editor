/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetresourcelist.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/metasprite/resourceitem.h"
#include "gui-qt/metasprite/nullframesetresourceitem.h"
#include "gui-qt/metasprite/spriteimporter/resourceitem.h"
#include "gui-qt/project.h"
#include "models/metasprite/metasprite-serializer.h"
#include "models/metasprite/spriteimporter-serializer.h"
#include "models/project/project.h"

#include <QFileInfo>

using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite;

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

FrameSetResourceList::FrameSetResourceList(Project* project)
    : AbstractResourceList(project, ResourceTypeIndex::MS_FRAMESET)
{
}

std::vector<UnTech::MetaSprite::FrameSetFile>& FrameSetResourceList::frameSetFiles() const
{
    return project()->projectFile()->frameSets;
}

const QString FrameSetResourceList::resourceTypeNameSingle() const
{
    return tr("Frameset");
}

const QString FrameSetResourceList::resourceTypeNamePlural() const
{
    return tr("FrameSets");
}

size_t FrameSetResourceList::nItems() const
{
    return frameSetFiles().size();
}

AbstractResourceItem* FrameSetResourceList::buildResourceItem(size_t index)
{
    auto& frameSets = frameSetFiles();

    Q_ASSERT(index < frameSets.size());
    auto& frameSet = frameSets.at(index);

    switch (frameSet.type) {
    case FrameSetType::UNKNOWN:
        return new NullFrameSetResourceItem(this, index);

    case FrameSetType::METASPRITE:
        return new MetaSprite::ResourceItem(this, index);

    case FrameSetType::SPRITE_IMPORTER:
        return new SpriteImporter::ResourceItem(this, index);
    }

    return new NullFrameSetResourceItem(this, index);
}

const QVector<AbstractResourceList::AddResourceSettings>& FrameSetResourceList::addResourceSettings() const
{
    const static QVector<AbstractResourceList::AddResourceSettings> settings = {
        { .title = tr("Add MetaSprite FrameSet"),
          .filter = QString::fromUtf8("MetaSprite FrameSet (*.utms)"),
          .extension = QString::fromUtf8("utms"),
          .canCreateFile = true },
        { .title = tr("Add Sprite Importer FrameSet"),
          .filter = QString::fromUtf8("SpriteImporter FrameSet (*.utsi)"),
          .extension = QString::fromUtf8("utsi"),
          .canCreateFile = true },
    };

    return settings;
}

AbstractMsResourceItem* FrameSetResourceList::findResource(const QString& name) const
{
    return qobject_cast<AbstractMsResourceItem*>(AbstractResourceList::findResource(name));
}

void FrameSetResourceList::do_addResource(int settingIndex, const std::string& filename)
{
    auto& frameSets = frameSetFiles();

    frameSets.emplace_back();
    auto& frameSet = frameSets.back();
    frameSet.filename = filename;

    switch ((SettingIndexes)settingIndex) {
    case METASPRITE_ITEM:
        frameSet.type = FrameSetType::METASPRITE;
        break;

    case SPRITEIMPORTER_ITEM:
        frameSet.type = FrameSetType::SPRITE_IMPORTER;
        break;
    }

    QFileInfo fi(QString::fromStdString(filename));
    if (fi.exists()) {
        return;
    }

    QString name = fi.baseName();
    IdstringValidator().fixup(name);

    switch ((SettingIndexes)settingIndex) {
    case METASPRITE_ITEM: {
        MS::FrameSet fs;
        fs.name = name.toStdString();
        MS::saveFrameSet(fs, filename);
    } break;

    case SPRITEIMPORTER_ITEM: {
        SI::FrameSet fs;
        fs.name = name.toStdString();
        SI::saveFrameSet(fs, filename);
    } break;
    }
}

void FrameSetResourceList::do_removeResource(unsigned index)
{
    auto& frameSets = frameSetFiles();

    Q_ASSERT(index < frameSets.size());
    frameSets.erase(frameSets.begin() + index);
}
