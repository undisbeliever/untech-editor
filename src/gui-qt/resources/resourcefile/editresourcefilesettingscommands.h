/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/resources/document.h"
#include "models/resources/resources.h"
#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace Resources {

namespace RES = UnTech::Resources;
namespace MT = UnTech::MetaTiles;

template <class DT>
class EditResourceFileSettingsCommand : public QUndoCommand {

    using DataT = DT;

public:
    EditResourceFileSettingsCommand(Document* document,
                                    const DataT& newValue,
                                    const QString& text,
                                    QUndoCommand* parent = Q_NULLPTR)
        : QUndoCommand(text, parent)
        , _document(document)
        , _oldValue(data())
        , _newValue(newValue)
    {
    }

    DataT& data();

    virtual void undo() final
    {
        data() = _oldValue;
        emit _document->resourceFileSettingsChanged();
    }

    virtual void redo() final
    {
        data() = _newValue;
        emit _document->resourceFileSettingsChanged();
    }

private:
    Document* const _document;
    const DataT _oldValue;
    const DataT _newValue;
};

template <>
inline RES::BlockSettings& EditResourceFileSettingsCommand<RES::BlockSettings>::data()
{
    return _document->resourcesFile()->blockSettings;
}

template <>
inline MT::EngineSettings& EditResourceFileSettingsCommand<MT::EngineSettings>::data()
{
    return _document->resourcesFile()->metaTileEngineSettings;
}
}
}
}
