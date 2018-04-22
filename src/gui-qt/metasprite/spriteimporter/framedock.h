/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include "models/metasprite/spriteimporter.h"
#include <QDockWidget>
#include <QItemSelection>
#include <memory>

namespace UnTech {
namespace GuiQt {
class PropertyTableModel;

namespace MetaSprite {
namespace SpriteImporter {
namespace Ui {
class FrameDock;
}
class Actions;
class Document;
class FrameObjectManager;
class ActionPointManager;
class EntityHitboxManager;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class FrameDock : public QDockWidget {
    Q_OBJECT

public:
    FrameDock(Accessor::IdmapListModel* frameListModel, Actions* actions,
              QWidget* parent = nullptr);
    ~FrameDock();

    void setDocument(Document* document);

    void clearGui();

private slots:
    void onSelectedFrameChanged();
    void onFrameComboBoxActivated();
    void updateFrameContentsSelection();

    void onFrameDataChanged(const void* frame);

    void updateGui();

    void onSpriteOrderEdited();
    void onFrameLocationEdited();
    void onSolidClicked();
    void onTileHitboxEdited();

    void onFrameContentsSelectionChanged();
    void onFrameContentsContextMenu(const QPoint& pos);

private:
    std::unique_ptr<Ui::FrameDock> const _ui;
    Accessor::IdmapListModel* const _frameListModel;
    Actions* const _actions;

    Document* _document;

    FrameObjectManager* const _frameObjectManager;
    ActionPointManager* const _actionPointManager;
    EntityHitboxManager* const _entityHitboxManager;
    PropertyTableModel* const _frameContentsModel;
};
}
}
}
}
