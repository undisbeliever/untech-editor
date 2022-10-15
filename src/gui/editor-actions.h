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

    class BaseAction : public EditorUndoAction {
    protected:
        EditorT* const editor;

    public:
        ~BaseAction() override = default;

    protected:
        BaseAction(EditorT* editor)
            : editor(editor)
        {
            assert(editor != nullptr);
        }

        EditorDataT& getProjectData(Project::ProjectFile& projectFile) const
        {
            EditorDataT* data = ActionPolicy::getEditorData(projectFile, editor->itemIndex());
            assert(data != nullptr);
            return *data;
        }

        EditorDataT& getEditorData() const
        {
            EditorDataT* data = ActionPolicy::getEditorData(*editor);
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
        const EditorDataT newValue;
        // set by firstDo()
        EditorDataT oldValue;

    public:
        EditDataAction(EditorT* editor)
            : BaseAction(editor)
            , newValue(this->getEditorData())
        {
        }
        virtual ~EditDataAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            EditorDataT& projectData = this->getProjectData(projectFile);

            oldValue = projectData;

            projectData = newValue;

            // operator!= may not implemented in a few of my structs
            return !(oldValue == newValue);
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            EditorDataT& projectData = this->getProjectData(projectFile);
            EditorDataT& editorData = this->getEditorData();

            projectData = oldValue;
            editorData = oldValue;
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            EditorDataT& projectData = this->getProjectData(projectFile);
            EditorDataT& editorData = this->getEditorData();

            projectData = newValue;
            editorData = newValue;
        }
    };

    template <auto FieldPtr>
    class EditFieldAction final : public BaseAction {
        static_assert(std::is_member_object_pointer_v<decltype(FieldPtr)>);

        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        const FieldT newValue;
        // set by firstDo()
        FieldT oldValue;

    public:
        EditFieldAction(EditorT* editor)
            : BaseAction(editor)
            , newValue((this->getEditorData()).*FieldPtr)
        {
        }
        virtual ~EditFieldAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            EditorDataT& projectData = this->getProjectData(projectFile);

            oldValue = projectData.*FieldPtr;

            projectData.*FieldPtr = newValue;

            // operator!= may not implemented in a few of my structs
            return !(oldValue == newValue);
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            EditorDataT& projectData = this->getProjectData(projectFile);
            EditorDataT& editorData = this->getEditorData();

            projectData.*FieldPtr = oldValue;
            editorData.*FieldPtr = oldValue;
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            EditorDataT& projectData = this->getProjectData(projectFile);
            EditorDataT& editorData = this->getEditorData();

            projectData.*FieldPtr = newValue;
            editorData.*FieldPtr = newValue;
        }
    };

    static void editorDataEdited(EditorT* editor)
    {
        editor->addAction(
            std::make_unique<EditDataAction>(editor));
    }

    template <auto FieldPtr>
    static void fieldEdited(EditorT* editor)
    {
        editor->addAction(
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

    class EditFieldAction : public EditorUndoAction {
    private:
        EditorT* const editor;

        const FieldT newValue;
        // set by firstDo()
        FieldT oldValue;

        FieldT& getProjectField(Project::ProjectFile& projectFile) const
        {
            EditorDataT* data = ActionPolicy::getEditorData(projectFile, editor->itemIndex());
            assert(data != nullptr);
            return data->*FieldPtr;
        }

        FieldT& getEditorField() const
        {
            EditorDataT* data = ActionPolicy::getEditorData(*editor);
            assert(data != nullptr);
            return data->*FieldPtr;
        }

    public:
        EditFieldAction(EditorT* editor)
            : editor(editor)
            , newValue(this->getEditorField())
        {
            assert(editor != nullptr);
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
            FieldT& projectData = this->getProjectField(projectFile);

            oldValue = projectData;

            projectData = newValue;

            // operator!= may not implemented in a few of my structs
            return !(oldValue == newValue);
        }

        void undo(Project::ProjectFile& projectFile) const final
        {
            FieldT& projectData = this->getProjectField(projectFile);
            FieldT& editorData = this->getEditorField();

            projectData = oldValue;
            editorData = oldValue;
        }

        void redo(Project::ProjectFile& projectFile) const final
        {
            FieldT& projectData = this->getProjectField(projectFile);
            FieldT& editorData = this->getEditorField();

            projectData = newValue;
            editorData = newValue;
        }
    };

    static void fieldEdited(EditorT* editor)
    {
        editor->addAction(
            std::make_unique<EditFieldAction>(editor));
    }
};

}
