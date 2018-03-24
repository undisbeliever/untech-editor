/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "mttilesetcentralwidget.h"
#include "mttilesetpropertymanager.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/abstracteditor.h"
#include "gui-qt/resources/genericpropertieswidget.h"

namespace UnTech {
namespace GuiQt {
namespace Resources {

class MtTilesetEditor : public AbstractEditor {
    Q_OBJECT

public:
    MtTilesetEditor(QWidget* parent, ZoomSettings* zoomSettings)
        : AbstractEditor(parent)
        , _editorWidget(new MtTilesetCentralWidget(parent, zoomSettings))
        , _propertyWidget(new GenericPropertiesWidget(
              new MtTilesetPropertyManager(parent),
              parent))
    {
    }
    ~MtTilesetEditor() = default;

    virtual bool setResourceItem(AbstractProject*, AbstractResourceItem* aItem)
    {
        MtTilesetResourceItem* item = qobject_cast<MtTilesetResourceItem*>(aItem);
        _editorWidget->setResourceItem(item);
        _propertyWidget->setResourceItem(item);

        return item != nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }
    virtual QWidget* propertyWidget() const final { return _propertyWidget; }

private:
    MtTilesetCentralWidget* const _editorWidget;
    GenericPropertiesWidget* const _propertyWidget;
};
}
}
}
