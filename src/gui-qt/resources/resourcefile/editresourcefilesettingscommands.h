/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/resources/resourceproject.h"
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
    EditResourceFileSettingsCommand(ResourceProject* project,
                                    const DataT& newValue,
                                    const QString& text,
                                    QUndoCommand* parent = Q_NULLPTR)
        : QUndoCommand(text, parent)
        , _project(project)
        , _oldValue(data())
        , _newValue(newValue)
    {
    }

    DataT& data();

    virtual void undo() final
    {
        data() = _oldValue;
        emit _project->resourceFileSettingsChanged();
    }

    virtual void redo() final
    {
        data() = _newValue;
        emit _project->resourceFileSettingsChanged();
    }

private:
    ResourceProject* const _project;
    const DataT _oldValue;
    const DataT _newValue;
};

template <>
inline RES::BlockSettings& EditResourceFileSettingsCommand<RES::BlockSettings>::data()
{
    return _project->resourcesFile()->blockSettings;
}

template <>
inline MT::EngineSettings& EditResourceFileSettingsCommand<MT::EngineSettings>::data()
{
    return _project->resourcesFile()->metaTileEngineSettings;
}
}
}
}
