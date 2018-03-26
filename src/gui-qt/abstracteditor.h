/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QWidget>

namespace UnTech {
namespace GuiQt {
class AbstractProject;
class AbstractResourceItem;

class AbstractEditor : public QObject {
    Q_OBJECT

public:
    AbstractEditor(QWidget* parent)
        : QObject(parent)
        , _parentWindow(parent)
    {
    }
    ~AbstractEditor() = default;

    QWidget* parentWindow() const { return _parentWindow; }

    virtual bool setResourceItem(AbstractProject* project, AbstractResourceItem* item) = 0;

    virtual QWidget* editorWidget() const = 0;
    virtual QWidget* propertyWidget() const { return nullptr; }

private:
    QWidget* const _parentWindow;
};
}
}