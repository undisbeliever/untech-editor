/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "undostack.h"
#include "abstract-editor.h"
#include "models/common/iterators.h"
#include <cassert>

namespace UnTech::Gui {

class MacroAction final : public UndoAction {
private:
    std::vector<std::unique_ptr<UndoAction>> actions;

public:
    MacroAction()
        : UndoAction()
    {
    }
    ~MacroAction() override = default;

    void addAction(std::unique_ptr<UndoAction>&& a)
    {
        assert(a != nullptr);
        actions.push_back(std::move(a));
    }

    void firstDo_editorData() const final
    {
        for (auto& a : actions) {
            a->firstDo_editorData();
        }
    }

    bool firstDo_projectFile(Project::ProjectFile& projectFile) final
    {
        bool changed = false;
        for (auto& a : actions) {
            assert(a != nullptr);

            const bool c = a->firstDo_projectFile(projectFile);
            changed |= c;
            if (!c) {
                a = nullptr;
            }
        }
        return changed;
    }

    void undo(Project::ProjectFile& projectFile) const final
    {
        for (const auto& a : reverse(actions)) {
            if (a) {
                a->undo(projectFile);
            }
        }
    }

    void redo(Project::ProjectFile& projectFile) const final
    {
        for (auto& a : actions) {
            if (a) {
                a->redo(projectFile);
            }
        }
    }

    void notifyGui(AbstractEditorGui* gui) const final
    {
        for (auto& a : actions) {
            if (a) {
                a->notifyGui(gui);
            }
        }
    }
};

void trimStack(std::vector<std::unique_ptr<UndoAction>>& stack)
{
    while (stack.size() > UndoStack::N_UNDO_ACTIONS) {
        stack.erase(stack.begin());
    }
}

void UndoStack::addAction(std::unique_ptr<UndoAction>&& action)
{
    if (_inMacro == false) {
        _pendingEditorActions.push_back(std::move(action));
    }
    else {
        assert(!_pendingEditorActions.empty());
        if (!_pendingEditorActions.empty()) {
            auto* m = dynamic_cast<MacroAction*>(_pendingEditorActions.back().get());
            assert(m);
            if (m) {
                m->addAction(std::move(action));
            }
        }
    }
}

void UndoStack::startMacro()
{
    assert(_inMacro == false);
    if (_inMacro == false) {
        _pendingEditorActions.push_back(std::make_unique<MacroAction>());
        _inMacro = true;
    }
}

void UndoStack::endMacro()
{
    _inMacro = false;
}

void UndoStack::processEditorActions(AbstractEditorGui* gui)
{
    const auto pendingSize = _pendingEditorActions.size();
    const auto undoSize = _undoStack.size();
    const auto redoSize = _redoStack.size();

    assert(_inMacro == false);
    _inMacro = false;

    if (!_pendingEditorActions.empty()) {
        _clean = false;
    }

    for (auto& pa : _pendingEditorActions) {
        auto action = std::move(pa);

        action->firstDo_editorData();
        action->notifyGui(gui);

        assert(_undoStack.size() == undoSize && "UndoAction must not modify the undo stack");
        assert(_redoStack.size() == redoSize && "UndoAction must not modify the undo stack");
        assert(_pendingEditorActions.size() == pendingSize && "UndoAction must not call addAction");

        _pendingProjectFileActions.push_back(std::move(action));
    }

    _pendingEditorActions.clear();
}

bool UndoStack::processPendingProjectActions(Project::ProjectFile& projectFile)
{
    assert(_pendingEditorActions.empty());

    bool edited = false;

    assert(_inMacro == false);
    _inMacro = false;

    for (auto& pa : _pendingProjectFileActions) {
        auto action = std::move(pa);

        const auto undoSize = _undoStack.size();
        const auto redoSize = _redoStack.size();

        const bool modifed = action->firstDo_projectFile(projectFile);
        _clean = false;
        edited = true;

        assert(_undoStack.size() == undoSize && "UndoAction must not modify the undo stack");
        assert(_redoStack.size() == redoSize && "UndoAction must not modify the undo stack");
        assert(_pendingEditorActions.empty() && "UndoAction must not call addAction");

        if (modifed) {
            _undoStack.push_back(std::move(action));
            _redoStack.clear();
        }
    }

    _pendingProjectFileActions.clear();

    trimStack(_undoStack);

    return edited;
}

bool UndoStack::undo(Project::ProjectFile& projectFile, AbstractEditorGui* gui)
{
    // Assumes AbstractEditorData matches projectFile data
    assert(_pendingEditorActions.empty());

    if (_undoStack.empty()) {
        return false;
    }
    if (!_pendingProjectFileActions.empty()) {
        return false;
    }

    auto a = std::move(_undoStack.back());
    _undoStack.pop_back();

    const auto undoSize = _undoStack.size();
    const auto redoSize = _redoStack.size();

    a->undo(projectFile);
    a->notifyGui(gui);
    _clean = false;

    assert(_undoStack.size() == undoSize && "UndoAction must not modify the undo stack");
    assert(_redoStack.size() == redoSize && "UndoAction must not modify the undo stack");
    assert(_pendingEditorActions.empty() && "UndoAction must not call addAction");

    _redoStack.push_back(std::move(a));
    trimStack(_redoStack);

    return true;
}

bool UndoStack::redo(Project::ProjectFile& projectFile, AbstractEditorGui* gui)
{
    // Assumes AbstractEditorData matches projectFile data
    assert(_pendingEditorActions.empty());

    if (_redoStack.empty()) {
        return false;
    }
    if (!_pendingProjectFileActions.empty()) {
        return false;
    }

    auto a = std::move(_redoStack.back());
    _redoStack.pop_back();

    const auto undoSize = _undoStack.size();
    const auto redoSize = _redoStack.size();

    a->redo(projectFile);
    a->notifyGui(gui);
    _clean = false;

    assert(_undoStack.size() == undoSize && "UndoAction must not modify the undo stack");
    assert(_redoStack.size() == redoSize && "UndoAction must not modify the undo stack");
    assert(_pendingEditorActions.empty() && "UndoAction must not call addAction");

    _undoStack.push_back(std::move(a));
    trimStack(_undoStack);

    return true;
}

bool processUndoStack(AbstractEditorData* editor, AbstractEditorGui* gui, Project::ProjectFile& pf)
{
    assert(editor);

    bool edited = false;

    // Ensure AbstractEditorData and ProjectFile data are in sync
    editor->undoStack().processEditorActions(gui);
    edited |= editor->undoStack().processPendingProjectActions(pf);

    if (gui) {
        if (gui->undoClicked || gui->redoClicked) {
            // Discard any uncommitted changes
            editor->loadDataFromProject(pf);
        }

        if (gui->undoClicked) {
            edited = editor->undoStack().undo(pf, gui);
        }
        else if (gui->redoClicked) {
            edited = editor->undoStack().redo(pf, gui);
        }

        gui->undoClicked = false;
        gui->redoClicked = false;
    }

    // undo/redo may have modified the selection
    if (edited) {
        editor->updateSelection();
    }

    return edited;
}

}
