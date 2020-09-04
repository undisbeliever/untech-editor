/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-editor.h"
#include "gui/common/type-traits.h"
#include "models/common/externalfilelist.h"
#include "models/common/namedlist.h"

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
            return list->at(index);
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
    if (list) {
        if (index < list->size()) {
            auto& item = list->item(index);
            return { item.value.get(), item.filename };
        }
    }
    return { nullptr, {} };
}

template <typename ActionPolicy>
struct EditorActions {
    using EditorT = typename ActionPolicy::EditorT;
    using EditorDataT = typename ActionPolicy::EditorDataT;

    class BaseAction : public EditorUndoAction {
    protected:
        EditorT* const editor;

    protected:
        BaseAction(EditorT* editor)
            : editor(editor)
        {
            assert(editor != nullptr);
        }
        virtual ~BaseAction() = default;

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
    };

    class EditDataAction final : public BaseAction {
    private:
        // set by firstDo()
        EditorDataT oldValue;
        EditorDataT newValue;

    public:
        EditDataAction(EditorT* editor)
            : BaseAction(editor)
        {
        }
        virtual ~EditDataAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            EditorDataT& projectData = this->getProjectData(projectFile);
            EditorDataT& editorData = this->getEditorData();

            oldValue = projectData;
            newValue = editorData;

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

    template <auto FieldPtr, typename = std::enable_if_t<std::is_member_object_pointer_v<decltype(FieldPtr)>>>
    class EditFieldAction final : public BaseAction {
        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        // set by firstDo()
        FieldT oldValue;
        FieldT newValue;

    public:
        EditFieldAction(EditorT* editor)
            : BaseAction(editor)
        {
        }
        virtual ~EditFieldAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            EditorDataT& projectData = this->getProjectData(projectFile);
            EditorDataT& editorData = this->getEditorData();

            oldValue = projectData.*FieldPtr;
            newValue = editorData.*FieldPtr;

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

}
