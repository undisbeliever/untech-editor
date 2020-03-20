/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "models/common/namedlist.h"
#include "models/resources/background-image.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace BackgroundImage {
class ResourceList;

namespace RES = UnTech::Resources;

class ResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

public:
    using DataT = RES::BackgroundImageInput;

public:
    ResourceItem(ResourceList* parent, size_t index);
    ~ResourceItem() = default;

    static QString typeName() { return tr("Background Image"); }

    inline const RES::BackgroundImageInput& backgroundImageInput() const { return _backgroundImages.at(index()); }
    const optional<const RES::BackgroundImageData&> compiledData() const;

    bool edit_setName(const idstring& name);
    bool edit_setBitDepth(const unsigned bitDepth);
    bool edit_setImageFilename(const std::filesystem::path& filename);
    bool edit_setConversionPalette(const idstring& paletteName);
    bool edit_setFirstPalette(unsigned firstPalette);
    bool edit_setNPalettes(unsigned nPalettes);
    bool edit_setDefaultOrder(bool defaultOrder);

private:
    friend class Accessor::ResourceItemUndoHelper<ResourceItem>;
    const RES::BackgroundImageInput* data() const { return &_backgroundImages.at(index()); }
    RES::BackgroundImageInput* dataEditable() { return &_backgroundImages.at(index()); }

    void updateExternalFiles();
    void updateDependencies();

protected:
    virtual bool compileResource(ErrorList& err) final;

signals:
    void imageFilanemeChanged();

private:
    NamedList<RES::BackgroundImageInput>& _backgroundImages;
};
}
}
}
}
