/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "palettecentralwidget.h"
#include "palettepropertymanager.h"
#include "paletteresourceitem.h"
#include "gui-qt/resources/abstracteditor.h"
#include "gui-qt/resources/genericpropertieswidget.h"

namespace UnTech {
namespace GuiQt {
namespace Resources {

class PaletteEditor : public AbstractEditor {
    Q_OBJECT

public:
    PaletteEditor(QWidget* parent)
        : AbstractEditor(parent)
        , _editorWidget(new PaletteCentralWidget(parent))
        , _propertyWidget(new GenericPropertiesWidget(
              new PalettePropertyManager(parent),
              parent))
    {
    }
    ~PaletteEditor() = default;

    virtual bool setResourceItem(AbstractProject*, AbstractResourceItem* aItem)
    {
        PaletteResourceItem* item = qobject_cast<PaletteResourceItem*>(aItem);
        _editorWidget->setResourceItem(item);
        _propertyWidget->setResourceItem(item);

        return item != nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }
    virtual QWidget* propertyWidget() const final { return _propertyWidget; }

private:
    PaletteCentralWidget* const _editorWidget;
    GenericPropertiesWidget* const _propertyWidget;
};
}
}
}
