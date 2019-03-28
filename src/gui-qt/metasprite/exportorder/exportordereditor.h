/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "editorwidget.h"
#include "exportorderresourceitem.h"
#include "gui-qt/abstracteditor.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ExportOrder {

class ExportOrderEditor : public AbstractEditor {
    Q_OBJECT

public:
    ExportOrderEditor(QWidget* parent)
        : AbstractEditor(parent)
        , _editorWidget(new EditorWidget(parent))
    {
    }
    ~ExportOrderEditor() = default;

    virtual bool setResourceItem(Project*, AbstractResourceItem* aItem)
    {
        ExportOrderResourceItem* item = qobject_cast<ExportOrderResourceItem*>(aItem);
        _editorWidget->setExportOrderResource(item);

        return item != nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }

private:
    EditorWidget* const _editorWidget;
};
}
}
}
}
