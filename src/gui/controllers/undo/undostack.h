/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <list>
#include <memory>
#include <sigc++/signal.h>
#include <stdexcept>

namespace UnTech {
namespace Controller {
namespace Undo {

struct ActionType {
    // The text to display in undoMessage/redoMessage.
    const std::string text;

    // If true then the action can be retrieved by the
    // UndoStack so it can be merged with the previous
    // undo action.
    const bool canMerge = false;
};

/**
 * A virtual class whose subclasses will contain enough state to
 * undo and redo all the given action.
 *
 * The subclass is not responsible for preforming the action, that will
 * be the responsibility of the function that initiates the subclass
 * (see the actionhelper.h macros)
 *
 * ActionType MUST EXIST throughout the life of the entire program.  A pointer
 * comparison is used by the UndoStack to determine if any two actions are the
 * same type and accessed by `UndoStack::getMergeAction`.
 */
class Action {
public:
    Action(const ActionType* type);
    virtual ~Action() = default;

    Action(const Action&) = delete;

    /** Called by UndoStack when user presses undo */
    virtual void undo() = 0;

    /** Called by UndoStack when user presses redo */
    virtual void redo() = 0;

    const ActionType* type() const { return _type; }

private:
    const ActionType* _type;
};

/**
 * A simple undo stack that holds the Action subclasses, ready for the
 * undo and redo functions.
 */
class UndoStack {
    const unsigned STACK_LIMIT = 250;

public:
    UndoStack();
    ~UndoStack() = default;

    void add_undo(std::unique_ptr<Action> action);

    /**
     * Prevent undostack from retrieving the mergable action.
     *
     * This should be called when the widget goes out of focus
     */
    void dontMergeNextAction();

    /**
     * Retrieves the Action on the top of the undoStack IF
     *  1) _dontMerge is False
     *  2) The topmost undo action is merge-able
     *  3) The topmost undo action is of the same type as `type`
     *     (done by pointer comparison)
     *
     * Returns `nullptr` if the next undo action is not merge-able with type.
     */
    Action* retrieveMergableAction(const ActionType* type);

    void undo();
    void redo();

    void clear();

    bool isDirty() const { return _dirty; }

    void markDirty();
    void markClean();

    inline bool canUndo() const { return !_undoStack.empty(); }
    inline bool canRedo() const { return !_redoStack.empty(); }

    const std::string& undoMessage() const;
    const std::string& redoMessage() const;

    sigc::signal<void>& signal_stackChanged() { return _signal_stackChanged; }
    sigc::signal<void>& signal_dirtyBitChanged() { return _signal_dirtyBitChanged; }

private:
    sigc::signal<void> _signal_stackChanged;
    sigc::signal<void> _signal_dirtyBitChanged;

    // using list instead of stack so I can delete from the end.
    std::list<std::unique_ptr<Action>> _undoStack;
    std::list<std::unique_ptr<Action>> _redoStack;
    bool _dirty;
    bool _dontMerge;
};
}
}
}
