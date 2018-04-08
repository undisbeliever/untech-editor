/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QWidget>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace Ui {
class ExportOrderEditorWidget;
}
class ExportOrderModel;
class ExportOrderResourceItem;

class ExportOrderEditorWidget : public QWidget {
    Q_OBJECT

public:
    ExportOrderEditorWidget(QWidget* parent = 0);
    ~ExportOrderEditorWidget();

    void setExportOrderResource(ExportOrderResourceItem* item);

private:
    void showEditorForCurrentIndex();
    void addExportName(bool isFrame);

private slots:
    void updateSelection();
    void onViewSelectionChanged();

    void onContextMenuRequested(const QPoint& pos);

    void onActionAddFrame();
    void onActionAddAnimation();
    void onActionAddAlternative();
    void onActionCloneSelected();
    void onActionRemoveSelected();

private:
    std::unique_ptr<Ui::ExportOrderEditorWidget> const _ui;
    ExportOrderModel* const _model;

    ExportOrderResourceItem* _exportOrder;
};
}
}
}
