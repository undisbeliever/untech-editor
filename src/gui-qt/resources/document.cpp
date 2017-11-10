/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "models/resources/resources-serializer.h"

#include <QMessageBox>

using namespace UnTech::GuiQt::Resources;

Document::Document(QObject* parent)
    : AbstractDocument(parent)
    , _resourcesFile(std::make_unique<RES::ResourcesFile>())
{
    initModels();
}

void Document::initModels()
{
}

const QString& Document::fileFilter() const
{
    static const QString FILTER = QString::fromUtf8(
        "MetaSprite Resources File (*.utres);;All Files (*)");

    return FILTER;
}

bool Document::saveDocumentFile(const QString& filename)
{
    try {
        RES::saveResourcesFile(*_resourcesFile, filename.toUtf8().data());
        return true;
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error Saving File"), ex.what());
        return false;
    }
}

bool Document::loadDocumentFile(const QString& filename)
{
    try {
        auto res = RES::loadResourcesFile(filename.toUtf8().data());
        if (res) {
            _resourcesFile = std::move(res);
            initModels();
            return true;
        }
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error Opening File"), ex.what());
    }
    return false;
}
