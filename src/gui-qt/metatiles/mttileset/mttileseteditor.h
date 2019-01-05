/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "mttilesetmainwindow.h"
#include "mttilesetpropertymanager.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/abstracteditor.h"

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {

class MtTilesetEditor : public AbstractEditor {
    Q_OBJECT

public:
    MtTilesetEditor(QWidget* parent, ZoomSettingsManager* zoomManager)
        : AbstractEditor(parent)
        , _editorWidget(new MtTilesetMainWindow(parent, zoomManager))
    {
    }
    ~MtTilesetEditor() = default;

    virtual bool setResourceItem(AbstractProject*, AbstractResourceItem* aItem)
    {
        MtTilesetResourceItem* item = qobject_cast<MtTilesetResourceItem*>(aItem);
        _editorWidget->setResourceItem(item);

        return item != nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }
    virtual QWidget* propertyWidget() const final { return nullptr; }

    virtual void populateMenu(QMenu* editMenu, QMenu* viewMenu) final
    {
        _editorWidget->populateMenu(editMenu, viewMenu);
    }

    virtual void onErrorDoubleClicked(const ErrorListItem& error) final
    {
        _editorWidget->onErrorDoubleClicked(error);
    }

    virtual ZoomSettings* zoomSettings() const final { return _editorWidget->zoomSettings(); }

private:
    MtTilesetMainWindow* const _editorWidget;
};
}
}
}
