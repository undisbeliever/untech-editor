/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "version.h"
#include "metasprite/metasprite/document.h"
#include "metasprite/metasprite/mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationDisplayName("UnTech MetaSprite GUI");
    app.setApplicationVersion(UNTECH_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "utms file", "[file]");
    parser.process(app);

    MainWindow* window = new MainWindow();
    window->show();

    const QStringList args = parser.positionalArguments();
    if (args.size() > 0) {
        window->setDocument(Document::loadDocument(args.first()));
    }

    return app.exec();
}
