/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "resourceitem.h"
#include "models/common/imagecache.h"

using namespace UnTech::GuiQt::Resources::Palette;

PalettePropertyManager::PalettePropertyManager(QObject* parent)
    : PropertyListManager(parent)
    , _palette(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;

    addProperty(tr("Name"), NAME, Type::IDSTRING);
    addProperty(tr("Image"), IMAGE_FILENAME, Type::FILENAME, QStringLiteral("PNG Image (*.png)"));
    addProperty(tr("Rows Per Frame"), ROWS_PER_FRAME, Type::UNSIGNED, 1, 16);
    addProperty(tr("Animation Delay"), ANIMATION_DELAY, Type::UNSIGNED, 0, 0x10000);
    addProperty(tr("Skip First Frame"), SKIP_FIRST_FRAME, Type::BOOLEAN);
}

void PalettePropertyManager::setResourceItem(ResourceItem* item)
{
    if (_palette == item) {
        return;
    }

    if (_palette) {
        _palette->disconnect(this);
    }
    _palette = item;

    setEnabled(_palette != nullptr);

    if (_palette) {
        connect(_palette, &ResourceItem::dataChanged,
                this, &PalettePropertyManager::dataChanged);
    }

    emit dataChanged();
}

void PalettePropertyManager::updateParameters(int id, QVariant& param1, QVariant& param2) const
{
    if (_palette == nullptr) {
        return;
    }

    if (id == ROWS_PER_FRAME) {
        const RES::PaletteInput& pal = _palette->paletteInput();
        const auto& paletteImage = ImageCache::loadPngImage(pal.paletteImageFilename);

        param1 = 1;
        param2 = qMin(paletteImage->size().height, 16U);
    }
}

QVariant PalettePropertyManager::data(int id) const
{
    if (_palette == nullptr) {
        return QVariant();
    }

    const RES::PaletteInput& pal = _palette->paletteInput();

    switch ((PropertyId)id) {
    case NAME:
        return QString::fromStdString(pal.name);

    case IMAGE_FILENAME:
        return QString::fromStdString(pal.paletteImageFilename);

    case ROWS_PER_FRAME:
        return pal.rowsPerFrame;

    case ANIMATION_DELAY:
        return pal.animationDelay;

    case SKIP_FIRST_FRAME:
        return pal.skipFirstFrame;
    }

    return QVariant();
}

bool PalettePropertyManager::setData(int id, const QVariant& value)
{
    Q_ASSERT(_palette);

    switch ((PropertyId)id) {
    case NAME:
        return _palette->editPalette_setName(value.toString().toStdString());

    case IMAGE_FILENAME:
        return _palette->editPalette_setImageFilename(value.toString().toStdString());

    case ROWS_PER_FRAME:
        return _palette->editPalette_setRowsPerFrame(value.toUInt());

    case ANIMATION_DELAY:
        return _palette->editPalette_setAnimationDelay(value.toUInt());

    case SKIP_FIRST_FRAME:
        return _palette->editPalette_setSkipFirstFrame(value.toBool());
    }

    return false;
}
