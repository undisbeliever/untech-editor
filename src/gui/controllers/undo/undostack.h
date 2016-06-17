#pragma once

#include <list>
#include <memory>
#include <sigc++/signal.h>

namespace UnTech {
namespace Controller {
namespace Undo {

/**
 * A virtual class whose subclasses will contain enough state to
 * undo and redo all the given action.
 *
 * The subclass is not responsible for preforming the action, that will
 * be the responsibility of the function that initiates the subclass
 * (see the actionhelper.h macros)
 */
class Action {
public:
    virtual ~Action() = default;

    /** Called by UndoStack when user presses undo */
    virtual void undo() = 0;

    /** Called by UndoStack when user presses redo */
    virtual void redo() = 0;

    virtual const std::string& message() const = 0;
};

/**
 * A virtual class whose subclasses can be merged to combine
 * undo actions
 */
class MergeAction : public Action {
public:
    virtual ~MergeAction() = default;

    /** Attempts to merges the other action with this one.
     * Returns true if successful. The undostack will will delete it
     */
    virtual bool mergeWith(MergeAction* other) = 0;
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

    void add_undoMerge(std::unique_ptr<MergeAction> action);

    /**
     * Prevent action merging in the next add_undoMerge.
     *
     * This should be called when the widget goes out of focus
     */
    void dontMergeNextAction();

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

    sigc::signal<void> signal_stackChanged;
    sigc::signal<void> signal_dirtyBitChanged;

private:
    // using list instead of stack so I can delete from the end.
    std::list<std::unique_ptr<Action>> _undoStack;
    std::list<std::unique_ptr<Action>> _redoStack;
    bool _dirty;
    bool _dontMerge;
};
}
}
}
