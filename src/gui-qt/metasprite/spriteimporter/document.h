/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/spriteimporter.h"
#include <QObject>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {

namespace SI = UnTech::MetaSprite::SpriteImporter;

class Document : public QObject {
    Q_OBJECT

public:
    static const char* FILE_FILTER;

public:
    explicit Document(QObject* parent = nullptr);
    Document(std::unique_ptr<SI::FrameSet> frameSet,
             const QString& filename, QObject* parent = nullptr);
    ~Document() = default;

    static std::unique_ptr<Document> loadDocument(const QString& filename);
    bool saveDocument(const QString& filename);

    SI::FrameSet* frameSet() const { return _frameSet.get(); }
    const QString& filename() const { return _filename; }

private:
    std::unique_ptr<SI::FrameSet> _frameSet;
    QString _filename;
};
}
}
}
}
