/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QDialog>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog();

private slots:
    void onAboutQtClicked();

private:
    std::unique_ptr<Ui::AboutDialog> _ui;
};
}
}
