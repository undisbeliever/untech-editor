/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include "gui-qt/accessor/listaccessortablemanager.h"
#include "gui-qt/common/properties/propertylistmanager.h"
#include "models/metasprite/spriteimporter.h"
#include <QStringList>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;
class FrameList;
class AbstractFrameContentAccessor;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class FrameSetManager : public PropertyListManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        TILESET_TYPE,
        EXPORT_ORDER,
        IMAGE_FILENAME,
        TRANSPARENT_COLOR,
        GRID_FRAME_SIZE,
        GRID_OFFSET,
        GRID_PADDING,
        GRID_ORIGIN,
        USER_SUPPLIED_PALETTE,
        PALETTE_N_PALLETES,
        PALETTE_COLOR_SIZE,
    };

public:
    explicit FrameSetManager(QObject* parent = nullptr);
    ~FrameSetManager() = default;

    void setDocument(Document* document);

    virtual QVariant data(int id) const final;
    virtual void updateParameters(int id, QVariant& param1, QVariant& param2) const final;
    virtual bool setData(int id, const QVariant& value) final;

private:
    Document* _document;
};

class FrameManager : public PropertyListManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        SPRITE_ORDER,
        LOCATION_USE_GRID_LOCATION,
        LOCATION_GRID_LOCATION,
        LOCATION_AABB,
        LOCATION_USE_GRID_ORGIN,
        LOCATION_ORIGIN,
        SOLID,
        TILE_HITBOX,
    };

public:
    explicit FrameManager(QObject* parent = nullptr);
    ~FrameManager() = default;

    void setDocument(Document* document);

    virtual QVariant data(int id) const final;
    virtual void updateParameters(int id, QVariant& param1, QVariant& param2) const final;
    virtual bool setData(int id, const QVariant& value) final;

private slots:
    void onSelectedFrameChanged();
    void onFrameDataChanged(size_t frameIndex);

private:
    FrameList* _frameList;
};

class FrameObjectManager : public Accessor::ListAccessorTableManager {
    Q_OBJECT

public:
    enum PropertyId {
        LOCATION,
        SIZE,
    };
    static const QStringList SIZE_STRINGS;

private:
    Document* _document;

public:
    explicit FrameObjectManager(QObject* parent = nullptr);
    ~FrameObjectManager() = default;

    void setDocument(Document* document);

    virtual QVariant data(int index, int id) const final;
    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

private:
    const SI::Frame* selectedFrame() const;
};

class ActionPointManager : public Accessor::ListAccessorTableManager {
    Q_OBJECT

public:
    enum PropertyId {
        LOCATION,
        PARAMETER,
    };

private:
    Document* _document;

public:
    explicit ActionPointManager(QObject* parent = nullptr);
    ~ActionPointManager() = default;

    void setDocument(Document* document);

    virtual QVariant data(int index, int id) const final;
    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

private:
    const SI::Frame* selectedFrame() const;
};

class EntityHitboxManager : public Accessor::ListAccessorTableManager {
    Q_OBJECT

public:
    enum PropertyId {
        AABB,
        HITBOX_TYPE,
    };

private:
    Document* _document;

public:
    explicit EntityHitboxManager(QObject* parent = nullptr);
    ~EntityHitboxManager() = default;

    void setDocument(Document* document);

    virtual QVariant data(int index, int id) const final;
    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;

private:
    const SI::Frame* selectedFrame() const;
};
}
}
}
}
