/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "aboutdialog.h"
#include "version.h"
#include "gui-qt/common/aboutdialog.ui.h"

#include <QMessageBox>

using namespace UnTech::GuiQt;

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::AboutDialog)
{
    _ui->setupUi(this);

    _ui->about->setText(
        QString::fromUtf8(
            "<h1>%1</h1>"
            "<p>Version " UNTECH_VERSION "</p>"
            "<p>Part of the " UNTECH_NAME " Editor Suite</p>"
            "<p><a href=\"" UNTECH_URL "\">Website</a></p>"
            "<p><a href=\"" UNTECH_LICENSE_URL "\">Licensed Under " UNTECH_LICENSE "</a></p>"
            "<p>" UNTECH_COPYRIGHT "</p>"
            "<p>This program comes with absolutely no warranty.</p>")
            .arg(QApplication::applicationDisplayName()));

    _ui->thirdParty->setText(
        QString::fromUtf8(
            "<h2>Third Party Libraries:</h2>"
            "<table cellpadding=4 style=\"margin-left: 12px; margin-right: 12px\">"
            " <tr>"
            "  <td><a href=\"http://lodev.org/lodepng/\"><b>LodePNG</b></a></td>"
            "  <td>Copyright (c) 2005-2019 Lode Vandevenne,"
            "   <br/><a href=\"https://opensource.org/licenses/Zlib\">zlib License</a></td>"
            " </tr>"
            " <tr>"
            "  <td><a href=\"https://lz4.github.io/lz4//\"><b>LZ4 Library</b></a></td>"
            "  <td>Copyright (c) 2011-2018, Yann Collet,"
            "   <br/><a href=\"https://github.com/lz4/lz4/blob/master/lib/LICENSE\">BSD 2-Clause License</a></td>"
            " </tr>"
            " <tr>"
            "  <td><a href=\"https://www.qt.io/\"><b>The Qt Toolkit</b></a></td>"
            "  <td>Copyright (c) 2017 The Qt Company Ltd, and other contributors,"
            "   <br/><a href=\"https://doc.qt.io/qt-5/lgpl.html\">LGPL version 3 License</a></td>"
            " </tr>"
            "</table>"));

    connect(_ui->closeButton, &QPushButton::clicked,
            this, &AboutDialog::accept);
    connect(_ui->aboutQtButton, &QPushButton::clicked,
            this, &AboutDialog::onAboutQtClicked);
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::onAboutQtClicked()
{
    QMessageBox::aboutQt(this);
}
