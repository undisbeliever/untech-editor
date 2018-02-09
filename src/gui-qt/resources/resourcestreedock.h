/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QDockWidget>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace Ui {
class ResourcesTreeDock;
}
class Document;
class ResourcesTreeModel;

class ResourcesTreeDock : public QDockWidget {
    Q_OBJECT

public:
    ResourcesTreeDock(QWidget* parent = nullptr);
    ~ResourcesTreeDock();

    void setDocument(Document* document);

private:
    std::unique_ptr<Ui::ResourcesTreeDock> _ui;
    ResourcesTreeModel* _model;

    Document* _document;
};
}
}
}
