/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include <QObject>
#include <QUndoStack>
#include <set>

namespace UnTech {
namespace GuiQt {
class AbstractIdmapListModel;

namespace MetaSprite {
struct SelectedItem;
class AbstractSelection;

class AbstractDocument : public QObject {
    Q_OBJECT

public:
    static const char* FILE_FILTER;

public:
    explicit AbstractDocument(QObject* parent = nullptr)
        : QObject(parent)
        , _filename()
        , _undoStack(new QUndoStack(this))
    {
    }

    ~AbstractDocument() = default;

    virtual bool saveDocument(const QString& filename) = 0;

    const QString& filename() const { return _filename; }
    QUndoStack* undoStack() const { return _undoStack; }

    virtual AbstractSelection* selection() const = 0;
    virtual AbstractIdmapListModel* frameListModel() const = 0;

signals:
    void frameSetDataChanged();
    void frameSetGridChanged();
    void frameDataChanged(const void* frame);
    void frameAdded(const void* frame);
    void frameAboutToBeRemoved(const void* frame);
    void frameRenamed(const void* frame, const idstring& newId);

    void frameObjectChanged(const void* frame, unsigned index);
    void actionPointChanged(const void* frame, unsigned index);
    void entityHitboxChanged(const void* frame, unsigned index);

    void frameObjectAboutToBeRemoved(const void* frame, unsigned index);
    void actionPointAboutToBeRemoved(const void* frame, unsigned index);
    void entityHitboxAboutToBeRemoved(const void* frame, unsigned index);

    void frameObjectAdded(const void* frame, unsigned index);
    void actionPointAdded(const void* frame, unsigned index);
    void entityHitboxAdded(const void* frame, unsigned index);

    void frameContentsMoved(const void* frame,
                            const std::set<SelectedItem>& oldPositions, int offset);

protected:
    QString _filename;
    QUndoStack* _undoStack;
};
}
}
}
