/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "document.h"
#include "mainwindow.h"
#include "gui-qt/abstracteditor.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {

class SiFrameSetEditor : public AbstractEditor {
    Q_OBJECT

public:
    SiFrameSetEditor(QWidget* parent, ZoomSettings* zoomSettings)
        : AbstractEditor(parent)
        , _editorWidget(new MainWindow(zoomSettings, parent))
    {
    }
    ~SiFrameSetEditor() = default;

    virtual bool setResourceItem(AbstractProject*, AbstractResourceItem* aItem)
    {
        Document* item = qobject_cast<Document*>(aItem);
        _editorWidget->setDocument(item);

        return item != nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }
    virtual QWidget* propertyWidget() const final { return nullptr; }

private:
    MainWindow* const _editorWidget;
};
}
}
}
}
