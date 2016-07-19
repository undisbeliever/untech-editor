#include "undostack.h"

using namespace UnTech::Controller::Undo;

UndoStack::UndoStack()
    : _undoStack()
    , _redoStack()
    , _dirty(false)
    , _dontMerge(false)
{
}

void UndoStack::add_undo(std::unique_ptr<Action> action)
{
    _undoStack.push_front(std::move(action));

    while (_undoStack.size() > STACK_LIMIT) {
        _undoStack.pop_back();
    }

    _dontMerge = false;

    _redoStack.clear();

    _signal_stackChanged.emit();

    markDirty();
}

void UndoStack::add_undoMerge(std::unique_ptr<MergeAction> actionToMerge)
{
    if (_undoStack.empty() || _dontMerge == true || canRedo()) {
        return add_undo(std::move(actionToMerge));
    }

    MergeAction* lastAction = dynamic_cast<MergeAction*>(
        _undoStack.front().get());

    if (lastAction == nullptr) {
        return add_undo(std::move(actionToMerge));
    }

    bool m = lastAction->mergeWith(actionToMerge.get());
    if (m) {
        markDirty();
    }
    else {
        add_undo(std::move(actionToMerge));
    }
}

void UndoStack::dontMergeNextAction()
{
    _dontMerge = true;
}

void UndoStack::undo()
{
    if (canUndo()) {
        _undoStack.front()->undo();

        _redoStack.splice(_redoStack.begin(), _undoStack, _undoStack.begin());

        while (_redoStack.size() > STACK_LIMIT) {
            _redoStack.pop_back();
        }

        _signal_stackChanged.emit();

        markDirty();
    }
}

void UndoStack::redo()
{
    if (canRedo()) {
        _redoStack.front()->redo();

        _undoStack.splice(_undoStack.begin(), _redoStack, _redoStack.begin());

        _signal_stackChanged.emit();
    }
}

void UndoStack::clear()
{
    _undoStack.clear();
    _redoStack.clear();

    _signal_stackChanged.emit();

    markClean();
}

void UndoStack::markDirty()
{
    if (_dirty != true) {
        _dirty = true;
        _signal_dirtyBitChanged.emit();
    }
}

void UndoStack::markClean()
{
    if (_dirty != false) {
        _dirty = false;
        _signal_dirtyBitChanged.emit();
    }
}

static const std::string emptyString;

const std::string& UndoStack::undoMessage() const
{
    if (!_undoStack.empty()) {
        return _undoStack.front()->message();
    }
    else {
        return emptyString;
    }
}

const std::string& UndoStack::redoMessage() const
{
    if (!_redoStack.empty()) {
        return _redoStack.front()->message();
    }
    else {
        return emptyString;
    }
}
