/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace Resources {

template <class ResourceItemT>
class EditResourceItemCommand : public QUndoCommand {

    using DataT = typename ResourceItemT::DataT;

public:
    EditResourceItemCommand(ResourceItemT* item,
                            const DataT& oldData,
                            const DataT& newData,
                            const QString& text,
                            QUndoCommand* parent = Q_NULLPTR)
        : QUndoCommand(text, parent)
        , _item(item)
        , _oldData(oldData)
        , _newData(newData)
    {
    }

    virtual void undo() final
    {
        _item->setData(_oldData);
    }

    virtual void redo() final
    {
        _item->setData(_newData);
    }

private:
    ResourceItemT* const _item;
    const DataT _oldData;
    const DataT _newData;
};
}
}
}
