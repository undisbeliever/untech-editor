/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "selection.h"
#include "gui-qt/common/properties/propertytablemanager.h"
#include "models/metasprite/spriteimporter.h"
#include <QStringList>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;

namespace SI = UnTech::MetaSprite::SpriteImporter;

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

protected:
    virtual SelectedItem::Type itemType() const = 0;

protected slots:
    void onSelectedFrameChanged();

    void onFrameLocationChanged(const void* frame);
    void onItemChanged(const void* frame, unsigned index);
    void onItemAdded(const void* frame, unsigned index);
    void onItemAboutToBeRemoved(const void* frame, unsigned index);

protected:
    Document* _document;
    SI::Frame* _frame;
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

protected:
    virtual SelectedItem::Type itemType() const final;
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

protected:
    virtual SelectedItem::Type itemType() const final;
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

protected:
    virtual SelectedItem::Type itemType() const final;
};
}
}
}
}
