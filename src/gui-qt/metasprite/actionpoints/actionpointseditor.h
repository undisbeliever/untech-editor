/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "actionpointsresourceitem.h"
#include "editorwidget.h"
#include "gui-qt/abstracteditor.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ActionPoints {

class ActionPointsEditor : public AbstractEditor {
    Q_OBJECT

public:
    ActionPointsEditor(QWidget* parent)
        : AbstractEditor(parent)
        , _editorWidget(new EditorWidget(parent))
    {
    }
    ~ActionPointsEditor() = default;

    virtual bool setResourceItem(Project*, AbstractResourceItem* aItem)
    {
        auto* item = qobject_cast<ActionPointsResourceItem*>(aItem);

        _editorWidget->setResourceItem(item);

        return item != nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }
    virtual QWidget* propertyWidget() const final { return nullptr; }

private:
    EditorWidget* const _editorWidget;
};

}
}
}
}
