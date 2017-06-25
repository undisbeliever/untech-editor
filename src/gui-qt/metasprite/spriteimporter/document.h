/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/spriteimporter.h"
#include <QObject>
#include <QUndoStack>
#include <memory>
#include <set>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
struct SelectedItem;
class Selection;
class FrameListModel;
class FrameContentsModel;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class Document : public QObject {
    Q_OBJECT

public:
    static const char* FILE_FILTER;

public:
    explicit Document(QObject* parent = nullptr);
    Document(std::unique_ptr<SI::FrameSet> frameSet,
             const QString& filename, QObject* parent = nullptr);
    ~Document() = default;

    static std::unique_ptr<Document> loadDocument(const QString& filename);
    bool saveDocument(const QString& filename);

    SI::FrameSet* frameSet() const { return _frameSet.get(); }
    const QString& filename() const { return _filename; }

    QUndoStack* undoStack() const { return _undoStack; }

    Selection* selection() const { return _selection; }
    FrameListModel* frameListModel() const { return _frameListModel; }
    FrameContentsModel* frameContentsModel() const { return _frameContentsModel; }

signals:
    void frameSetDataChanged();
    void frameSetGridChanged();
    void frameDataChanged(const SI::Frame*);
    void frameAdded(const SI::Frame*);
    void frameAboutToBeRemoved(const SI::Frame*);
    void frameRenamed(const SI::Frame*, const idstring& newId);

    void frameObjectChanged(const SI::Frame*, unsigned index);
    void actionPointChanged(const SI::Frame*, unsigned index);
    void entityHitboxChanged(const SI::Frame*, unsigned index);

    void frameObjectAboutToBeRemoved(const SI::Frame*, unsigned index);
    void actionPointAboutToBeRemoved(const SI::Frame*, unsigned index);
    void entityHitboxAboutToBeRemoved(const SI::Frame*, unsigned index);

    void frameObjectAdded(const SI::Frame*, unsigned index);
    void actionPointAdded(const SI::Frame*, unsigned index);
    void entityHitboxAdded(const SI::Frame*, unsigned index);

    void frameContentsMoved(const SI::Frame*,
                            const std::set<SelectedItem>& oldPositions, int offset);

private:
    std::unique_ptr<SI::FrameSet> _frameSet;
    QString _filename;

    QUndoStack* _undoStack;

    Selection* _selection;
    FrameListModel* _frameListModel;
    FrameContentsModel* _frameContentsModel;
};
}
}
}
}
