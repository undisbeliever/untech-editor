/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

template <class ProjectT>
class ProjectSettingsUndoHelper {

public:
    using DataT = typename ProjectT::DataT;

private:
    struct EmptySignalFunction {
        inline void operator()(ProjectT&) const {}
    };

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    class EditFieldCommand : public QUndoCommand {
    protected:
        ProjectT* const _project;
        const FieldT _oldValue;
        const FieldT _newValue;
        const UnaryFunction _getter;
        const ExtraSignalsFunction _signalEmitter;

    public:
        EditFieldCommand(ProjectT* item,
                         const FieldT& oldValue, const FieldT& newValue,
                         const QString& text,
                         UnaryFunction getter, ExtraSignalsFunction signalEmitter)
            : QUndoCommand(text)
            , _project(item)
            , _oldValue(oldValue)
            , _newValue(newValue)
            , _getter(getter)
            , _signalEmitter(signalEmitter)
        {
        }
        ~EditFieldCommand() = default;

        virtual void undo() final
        {
            setField(_oldValue);
        }

        virtual void redo() final
        {
            setField(_newValue);
        }

    private:
        inline void setField(const FieldT& value)
        {
            Q_ASSERT(_project);
            DataT* d = _project->dataEditable();
            Q_ASSERT(d);

            FieldT& f = _getter(*d);
            f = value;

            _signalEmitter(*_project);
            emit _project->resourceFileSettingsChanged();
        }
    };

private:
    ProjectT* const _project;

public:
    ProjectSettingsUndoHelper(ProjectT* project)
        : _project(project)
    {
        Q_ASSERT(project);
    }

public:
    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    QUndoCommand* editFieldCommand(const FieldT& newValue, const QString& text,
                                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        DataT* data = _project->dataEditable();
        if (data == nullptr) {
            return nullptr;
        }
        const FieldT& oldValue = getter(*data);

        if (oldValue == newValue) {
            return nullptr;
        }
        return new EditFieldCommand<FieldT, UnaryFunction, ExtraSignalsFunction>(
            _project, oldValue, newValue, text, getter, extraSignals);
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editField(const FieldT& newValue, const QString& text,
                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        QUndoCommand* c = editFieldCommand(newValue, text, getter, extraSignals);
        if (c) {
            _project->undoStack()->push(c);
        }
        return c != nullptr;
    }

    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* editFieldCommand(const FieldT& newValue, const QString& text,
                                   UnaryFunction getter)
    {
        return editFieldCommand(newValue, text, getter, EmptySignalFunction());
    }

    template <typename FieldT, typename UnaryFunction>
    bool editField(const FieldT& newValue, const QString& text,
                   UnaryFunction getter)
    {
        return editField(newValue, text, getter, EmptySignalFunction());
    }
};
}
}
}
