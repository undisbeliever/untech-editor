/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourcefilecentralwidget.h"
#include "resourcefilepropertieswidget.h"
#include "gui-qt/abstracteditor.h"
#include "gui-qt/genericpropertieswidget.h"
#include "gui-qt/resources/resourceproject.h"

namespace UnTech {
namespace GuiQt {
namespace Resources {

class ResourceFileEditor : public AbstractEditor {
    Q_OBJECT

public:
    ResourceFileEditor(QWidget* parent)
        : AbstractEditor(parent)
        , _editorWidget(new ResourceFileCentralWidget(parent))
        , _propertyWidget(new ResourceFilePropertiesWidget(parent))
    {
    }
    ~ResourceFileEditor() = default;

    virtual bool setResourceItem(AbstractProject* aProject, AbstractResourceItem* item)
    {
        ResourceProject* project = qobject_cast<ResourceProject*>(aProject);

        _editorWidget->setProject(project);
        _propertyWidget->setProject(project);

        // show this editor when no item is selected
        return project != nullptr && item == nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }
    virtual QWidget* propertyWidget() const final { return _propertyWidget; }

private:
    ResourceFileCentralWidget* const _editorWidget;
    ResourceFilePropertiesWidget* const _propertyWidget;
};
}
}
}
