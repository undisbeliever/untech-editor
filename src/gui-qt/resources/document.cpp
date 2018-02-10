/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "models/resources/resources-serializer.h"
#include "mttileset/mttilesetresourcelist.h"
#include "palette/paletteresourcelist.h"

#include <QMessageBox>

using namespace UnTech::GuiQt::Resources;

// _resourceLists order MUST match ResourceTypeIndex

Document::Document(QObject* parent)
    : AbstractDocument(parent)
    , _resourcesFile(std::make_unique<RES::ResourcesFile>())
    , _resourceLists({ {
          new PaletteResourceList(this, ResourceTypeIndex::PALETTE),
          new MtTilesetResourceList(this, ResourceTypeIndex::MT_TILESET),
      } })
    , _selectedResource(nullptr)
{
    initModels();
}

void Document::setSelectedResource(AbstractResourceItem* item)
{
    if (_selectedResource != item) {
        if (_selectedResource) {
            _selectedResource->disconnect(this);
        }

        _selectedResource = item;

        if (_selectedResource) {
            connect(_selectedResource, &QObject::destroyed,
                    this, &Document::onSelectedResourceDestroyed);
        }

        emit selectedResourceChanged();
    }
}

void Document::onSelectedResourceDestroyed(QObject* obj)
{
    if (_selectedResource == obj) {
        setSelectedResource(nullptr);
    }
}

void Document::initModels()
{
    for (auto& rl : _resourceLists) {
        rl->setDocument(this);
    }
}

const QString& Document::fileFilter() const
{
    static const QString FILTER = QString::fromUtf8(
        "MetaSprite Resources File (*.utres);;All Files (*)");

    return FILTER;
}

const QString& Document::defaultFileExtension() const
{
    static const QString EXTENSION = QString::fromUtf8("utres");
    return EXTENSION;
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