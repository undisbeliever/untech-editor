/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "version.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QSettings>

using namespace UnTech::GuiQt;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationDisplayName("UnTech Editor GUI");
    app.setOrganizationDomain(UNTECH_DOMAIN);
    app.setApplicationVersion(UNTECH_VERSION);

#ifdef Q_OS_WIN
    // Use an ini file instead of the registry to store GUI settings.
    //
    // The `window_state` QSettings key is ~5.3KB (as of December 2019) and
    // Microsoft's documentation recommends that we do not store values > 2048 bytes in the registry.
    // https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits
    QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

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
