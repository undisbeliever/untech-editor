/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractselection.h"
#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {

template <class DT>
class RaiseLowerFrameContents : public QUndoCommand {
    using DocumentT = DT;
    using FrameT = typename DocumentT::FrameT;

public:
    RaiseLowerFrameContents(DocumentT*, FrameT*,
                            const QString& text);
    ~RaiseLowerFrameContents() = default;

protected:
    void raiseItems(const std::set<SelectedItem>& items);
    void lowerItems(const std::set<SelectedItem>& items);

private:
    DocumentT* const _document;
    FrameT* const _frame;
};

template <class DT>
class RaiseFrameContents : public RaiseLowerFrameContents<DT> {
    using DocumentT = DT;
    using FrameT = typename DocumentT::FrameT;

public:
    RaiseFrameContents(DocumentT*, FrameT*,
                       const std::set<SelectedItem>& items);
    ~RaiseFrameContents() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    const std::set<SelectedItem> _undoItems;
    const std::set<SelectedItem> _redoItems;
};

template <class DT>
class LowerFrameContents : public RaiseLowerFrameContents<DT> {
    using DocumentT = DT;
    using FrameT = typename DocumentT::FrameT;

public:
    LowerFrameContents(DocumentT*, FrameT*,
                       const std::set<SelectedItem>& items);
    ~LowerFrameContents() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    const std::set<SelectedItem> _undoItems;
    const std::set<SelectedItem> _redoItems;
};
}
}
}
