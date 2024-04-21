/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-editor.h"
#include "editor-actions-notify-gui.h"
#include "models/common/externalfilelist.h"
#include "models/common/namedlist.h"
#include "models/common/type-traits.h"
#include <gsl/gsl>

namespace UnTech::Gui {

template <typename T>
T* namedListItem(NamedList<T>* list, const typename NamedList<T>::size_type index)
{
    if (list) {
        if (index < list->size()) {
            return &list->at(index);
        }
    }
    return nullptr;
}

template <typename T>
const T* namedListItem(const NamedList<T>* list, const typename NamedList<T>::size_type index)
{
    if (list) {
        if (index < list->size()) {
            return &list->at(index);
        }
    }
    return nullptr;
}

template <typename T>
T* fileListData(ExternalFileList<T>* list, const typename ExternalFileList<T>::size_type index)
{
    if (list) {
        if (index < list->size()) {
            return list->at(index).ptr();
        }
    }
    return nullptr;
}

template <typename T>
const T* fileListData(const ExternalFileList<T>* list, const typename ExternalFileList<T>::size_type index)
{
    if (list) {
        if (index < list->size()) {
            return list->at(index);
        }
    }
    return nullptr;
}

template <typename T>
const std::pair<const T*, const std::filesystem::path&>
fileListItem(const ExternalFileList<T>* list, const typename ExternalFileList<T>::size_type index)
{
    const static std::filesystem::path BLANK_FILENAME{};

    if (list) {
        if (index < list->size()) {
            auto& item = list->item(index);
            return { item.value.get(), item.filename };
        }
    }
    return { nullptr, BLANK_FILENAME };
}

template <typename ActionPolicy>
struct EditorActions {
    using EditorT = typename ActionPolicy::EditorT;
    using EditorDataT = typename ActionPolicy::EditorDataT;

    using NotNullEditorPtr = gsl::not_null<std::shared_ptr<EditorT>>;

    class BaseAction : public UndoAction {
    private:
        std::weak_ptr<EditorT> _editor;

    public:
        ~BaseAction() override = default;

    protected:
        BaseAction(const NotNullEditorPtr& editor)
            : UndoAction()
            , _editor(editor.get())
        {
        }

        [[nodiscard]] inline NotNullEditorPtr getEditor() const
        {
            auto e = _editor.lock();
            assert(e != nullptr);
            return e;
        }

        [[nodiscard]] EditorDataT& getProjectData(Project::ProjectFile& projectFile, EditorT& editor) const
        {
            EditorDataT* data = ActionPolicy::getEditorData(projectFile, editor.itemIndex());
            assert(data != nullptr);
            return *data;
        }

        [[nodiscard]] static EditorDataT& getEditorData(EditorT& editor)
        {
            EditorDataT* data = ActionPolicy::getEditorData(editor);
            assert(data != nullptr);
            return *data;
        }

    public:
        void notifyGui(AbstractEditorGui* gui) const final
        {
            editorUndoAction_notifyGui<ActionPolicy>(gui);
        }
    };

    class EditDataAction final : public BaseAction {
    private:
        EditorDataT newValue;
        // set by firstDo()
        EditorDataT oldValue;

    public:
        EditDataAction(const NotNullEditorPtr& editor)
            : BaseAction(editor)
            , newValue(this->getEditorData(*editor))
        {
        }
        virtual ~EditDataAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            EditorDataT& projectData = this->getProjectData(projectFile, *e);

            oldValue = projectData;

            projectData = newValue;

            // operator!= may not implemented in a few of my structs
            return !(oldValue == newValue);
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            EditorDataT& projectData = this->getProjectData(projectFile, *e);
            EditorDataT& editorData = this->getEditorData(*e);

            projectData = oldValue;
            editorData = oldValue;
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            EditorDataT& projectData = this->getProjectData(projectFile, *e);
            EditorDataT& editorData = this->getEditorData(*e);

            projectData = newValue;
            editorData = newValue;
        }
    };

    template <auto FieldPtr>
    class EditFieldAction final : public BaseAction {
        static_assert(std::is_member_object_pointer_v<decltype(FieldPtr)>);

        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        FieldT newValue;
        // set by firstDo()
        FieldT oldValue;

    public:
        EditFieldAction(const NotNullEditorPtr& editor)
            : BaseAction(editor)
            , newValue((this->getEditorData(*editor)).*FieldPtr)
        {
        }
        virtual ~EditFieldAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            EditorDataT& projectData = this->getProjectData(projectFile, *e);

            oldValue = projectData.*FieldPtr;

            projectData.*FieldPtr = newValue;

            // operator!= may not implemented in a few of my structs
            return !(oldValue == newValue);
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            EditorDataT& projectData = this->getProjectData(projectFile, *e);
            EditorDataT& editorData = this->getEditorData(*e);

            projectData.*FieldPtr = oldValue;
            editorData.*FieldPtr = oldValue;
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            EditorDataT& projectData = this->getProjectData(projectFile, *e);
            EditorDataT& editorData = this->getEditorData(*e);

            projectData.*FieldPtr = newValue;
            editorData.*FieldPtr = newValue;
        }
    };

    static void editorDataEdited(const NotNullEditorPtr& editor)
    {
        editor->undoStack().addAction(
            std::make_unique<EditDataAction>(editor));
    }

    template <auto FieldPtr>
    static void fieldEdited(const NotNullEditorPtr& editor)
    {
        editor->undoStack().addAction(
            std::make_unique<EditFieldAction<FieldPtr>>(editor));
    }
};

template <typename ActionPolicy>
struct EditorFieldActions {
    static_assert(std::is_member_object_pointer_v<decltype(ActionPolicy::FieldPtr)>);

    using EditorT = typename ActionPolicy::EditorT;
    using EditorDataT = typename ActionPolicy::EditorDataT;
    static constexpr auto FieldPtr = ActionPolicy::FieldPtr;
    using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    using NotNullEditorPtr = gsl::not_null<std::shared_ptr<EditorT>>;

    class EditFieldAction : public UndoAction {
    private:
        std::weak_ptr<EditorT> _editor;

        FieldT newValue;
        // set by firstDo()
        FieldT oldValue;

        [[nodiscard]] inline NotNullEditorPtr getEditor() const
        {
            auto e = _editor.lock();
            assert(e != nullptr);
            return e;
        }

        [[nodiscard]] FieldT& getProjectField(Project::ProjectFile& projectFile, EditorT& editor) const
        {
            EditorDataT* data = ActionPolicy::getEditorData(projectFile, editor.itemIndex());
            assert(data != nullptr);
            return data->*FieldPtr;
        }

        [[nodiscard]] static FieldT& getEditorField(EditorT& editor)
        {
            EditorDataT* data = ActionPolicy::getEditorData(editor);
            assert(data != nullptr);
            return data->*FieldPtr;
        }

    public:
        EditFieldAction(const NotNullEditorPtr& editor)
            : UndoAction()
            , _editor(editor.get())
            , newValue(this->getEditorField(*editor))
        {
        }
        ~EditFieldAction() override = default;

        void notifyGui(AbstractEditorGui* gui) const final
        {
            editorUndoAction_notifyGui<ActionPolicy>(gui);
        }

        void firstDo_editorData() const final
        {
        }

        bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = getEditor();

            FieldT& projectData = this->getProjectField(projectFile, *e);

            oldValue = projectData;

            projectData = newValue;

            // operator!= may not implemented in a few of my structs
            return !(oldValue == newValue);
        }

        void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = getEditor();

            FieldT& projectData = this->getProjectField(projectFile, *e);
            FieldT& editorData = this->getEditorField(*e);

            projectData = oldValue;
            editorData = oldValue;
        }

        void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = getEditor();

            FieldT& projectData = this->getProjectField(projectFile, *e);
            FieldT& editorData = this->getEditorField(*e);

            projectData = newValue;
            editorData = newValue;
        }
    };

    static void fieldEdited(const NotNullEditorPtr& editor)
    {
        editor->undoStack().addAction(
            std::make_unique<EditFieldAction>(editor));
    }
};

}
