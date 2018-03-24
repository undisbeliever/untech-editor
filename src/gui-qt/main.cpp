/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "version.h"
#include <QApplication>
#include <QCommandLineParser>

using namespace UnTech::GuiQt;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationDisplayName("UnTech Editor GUI");
    app.setOrganizationDomain(UNTECH_DOMAIN);
    app.setApplicationVersion(UNTECH_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "utres file", "[file]");
    parser.process(app);

    MainWindow* window = new MainWindow();
    window->show();

    const QStringList args = parser.positionalArguments();
    if (args.size() > 0) {
        window->loadProject(args.first());
    }

    return app.exec();
}
