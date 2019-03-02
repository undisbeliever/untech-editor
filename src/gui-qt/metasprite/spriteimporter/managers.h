/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include "gui-qt/common/properties/propertylistmanager.h"
#include "gui-qt/common/properties/propertytablemanager.h"
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

/*
 * The Frame Content Managers are not responsible for creating/removing/moving
 * the items, that is still the responsibility of the Selection and Action classes.
 */

class AbstractFrameContentManager : public PropertyTableManager {
    Q_OBJECT

public:
    explicit AbstractFrameContentManager(QObject* parent = nullptr);
    ~AbstractFrameContentManager() = default;

    virtual void setDocument(Document* document);

    const SI::Frame* selectedFrame() const;

protected:
    void connectSignals(Accessor::AbstractListAccessor* accessor);

private slots:
    void onSelectedFrameChanged();

    void onFrameLocationChanged(size_t frameIndex);
    void onItemChanged(size_t frameIndex, size_t index);
    void onListChanged(size_t frameIndex);

protected:
    Document* _document;
};

class FrameObjectManager : public AbstractFrameContentManager {
    Q_OBJECT

public:
    enum PropertyId {
        LOCATION,
        SIZE,
    };
    static const QStringList SIZE_STRINGS;

public:
    explicit FrameObjectManager(QObject* parent = nullptr);
    ~FrameObjectManager() = default;

    void setDocument(Document* document);

    virtual int rowCount() const final;
    virtual QVariant data(int index, int id) const final;
    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;
};

class ActionPointManager : public AbstractFrameContentManager {
    Q_OBJECT

public:
    enum PropertyId {
        LOCATION,
        PARAMETER,
    };

public:
    explicit ActionPointManager(QObject* parent = nullptr);
    ~ActionPointManager() = default;

    void setDocument(Document* document);

    virtual int rowCount() const final;
    virtual QVariant data(int index, int id) const final;
    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;
};

class EntityHitboxManager : public AbstractFrameContentManager {
    Q_OBJECT

public:
    enum PropertyId {
        AABB,
        HITBOX_TYPE,
    };

public:
    explicit EntityHitboxManager(QObject* parent = nullptr);
    ~EntityHitboxManager() = default;

    void setDocument(Document* document);

    virtual int rowCount() const final;
    virtual QVariant data(int index, int id) const final;
    virtual void updateParameters(int index, int id, QVariant& param1, QVariant& param2) const final;
    virtual bool setData(int index, int id, const QVariant& value) final;
};
}
}
}
}
