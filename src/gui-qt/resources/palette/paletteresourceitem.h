/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../abstractresourceitem.h"
#include "../document.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Resources {

class PaletteResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

public:
    using DataT = RES::PaletteInput;

public:
    PaletteResourceItem(AbstractResourceList* parent, size_t index);
    ~PaletteResourceItem() = default;

    virtual QString name() const final;

    // will never be null
    inline const RES::PaletteInput* paletteData() const
    {
        return document()->resourcesFile()->palettes.at(index()).get();
    }

protected:
    template <class T>
    friend class EditResourceItemCommand;
    void setData(const RES::PaletteInput& data);

protected:
    virtual bool compileResource(RES::ErrorList& err) final;

signals:
    void imageFilenameChanged();

private:
    inline const auto& palettesData() const
    {
        return document()->resourcesFile()->palettes;
    }
};
}
}
}
