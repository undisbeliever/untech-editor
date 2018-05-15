/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetresourcelist.h"
#include "metaspriteproject.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/metasprite/document.h"
#include "gui-qt/metasprite/nullframesetresourceitem.h"
#include "gui-qt/metasprite/spriteimporter/document.h"
#include "models/metasprite/metasprite-serializer.h"
#include "models/metasprite/spriteimporter-serializer.h"

#include <QFileInfo>

using FrameSetType = UnTech::MetaSprite::Project::FrameSetType;
using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite;

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

FrameSetResourceList::FrameSetResourceList(MetaSpriteProject* project)
    : AbstractResourceList(project, ResourceTypeIndex::MS_FRAMESET)
{
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
    return project()->metaSpriteProject()->frameSets.size();
}

AbstractResourceItem* FrameSetResourceList::buildResourceItem(size_t index)
{
    auto& frameSets = project()->metaSpriteProject()->frameSets;

    Q_ASSERT(index < frameSets.size());
    auto& frameSet = frameSets.at(index);

    switch (frameSet.type) {
    case FrameSetType::NONE:
    case FrameSetType::UNKNOWN:
        return new NullFrameSetResourceItem(this, index);

    case FrameSetType::METASPRITE:
        return new MetaSprite::Document(this, index);

    case FrameSetType::SPRITE_IMPORTER:
        return new SpriteImporter::Document(this, index);
    }

    return new NullFrameSetResourceItem(this, index);
    ;
}

const QList<AbstractResourceList::AddResourceSettings>& FrameSetResourceList::addResourceSettings() const
{
    const static QList<AbstractResourceList::AddResourceSettings> settings = {
        { tr("Add MetaSprite FrameSet"),
          QString::fromUtf8("MetaSprite FrameSet (*.utms)"),
          QString::fromUtf8("utms"),
          true },
        { tr("Add Sprite Importer FrameSet"),
          QString::fromUtf8("SpriteImporter FrameSet (*.utsi)"),
          QString::fromUtf8("utsi"),
          true },
    };

    return settings;
}

void FrameSetResourceList::do_addResource(int settingIndex, const std::string& filename)
{
    auto& frameSets = project()->metaSpriteProject()->frameSets;

    frameSets.emplace_back();
    auto& frameSet = frameSets.back();
    frameSet.filename = filename;
    frameSet.type = FrameSetType::NONE;

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
    auto& frameSets = project()->metaSpriteProject()->frameSets;

    Q_ASSERT(index < frameSets.size());
    frameSets.erase(frameSets.begin() + index);
}
