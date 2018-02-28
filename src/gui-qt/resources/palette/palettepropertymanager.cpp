/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettepropertymanager.h"
#include "paletteresourceitem.h"

using namespace UnTech::GuiQt::Resources;

PalettePropertiesManager::PalettePropertiesManager(QObject* parent)
    : AbstractPropertyManager(parent)
    , _palette(nullptr)
{
    addProperty(tr("Name"), NAME);
    addProperty(tr("Image"), IMAGE_FILENAME);
    addProperty(tr("Rows Per Frame"), ROWS_PER_FRAME);
    addProperty(tr("Animation Delay"), ANIMATION_DELAY);
    addProperty(tr("Skip First Frame"), SKIP_FIRST_FRAME);
}

ResourceTypeIndex PalettePropertiesManager::resourceTypeIndex() const
{
    return ResourceTypeIndex::PALETTE;
}

void PalettePropertiesManager::setResourceItem(AbstractResourceItem* abstractItem)
{
    PaletteResourceItem* item = qobject_cast<PaletteResourceItem*>(abstractItem);

    if (_palette != item) {
        _palette = item;

        setEnabled(_palette != nullptr);

        emit dataChanged();
    }
}

QVariant PalettePropertiesManager::data(int id) const
{
    if (_palette == nullptr) {
        return QVariant();
    }

    const std::shared_ptr<RES::PaletteInput> pal = _palette->paletteData();
    if (pal == nullptr) {
        return QVariant();
    }

    switch ((PropertyId)id) {
    case NAME:
        return QString::fromStdString(pal->name);

    case IMAGE_FILENAME:
        return QString::fromStdString(pal->paletteImageFilename);

    case ROWS_PER_FRAME:
        return pal->rowsPerFrame;

    case ANIMATION_DELAY:
        return pal->animationDelay;

    case SKIP_FIRST_FRAME:
        return pal->skipFirstFrame;
    }

    return QVariant();
}

bool PalettePropertiesManager::setData(int id, const QVariant& value)
{
    Q_ASSERT(_palette);
    const auto palleteInput = _palette->paletteData();
    Q_ASSERT(palleteInput);

    // ::TODO QUndoCommand::
    auto& newData = *_palette->paletteData();

    switch ((PropertyId)id) {
    case NAME:
        newData.name = value.toString().toStdString();
        break;

    case IMAGE_FILENAME:
        newData.paletteImageFilename = value.toString().toStdString();
        newData.paletteImage.loadPngImage(newData.paletteImageFilename);
        break;

    case ROWS_PER_FRAME:
        newData.rowsPerFrame = value.toUInt();
        break;

    case ANIMATION_DELAY:
        newData.animationDelay = value.toUInt();
        break;

    case SKIP_FIRST_FRAME:
        newData.skipFirstFrame = value.toBool();
        break;
    }

    return true;
}
