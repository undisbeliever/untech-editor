/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "filedialogs.h"
#include "models/common/file.h"

using namespace UnTech;

wxString View::generateWildcard(const DocumentType& type)
{
    wxString wildcard = type.name;
    wildcard << " (*." << type.extension << ")|*." << type.extension
             << "|All Files |" << wxFileSelectorDefaultWildcardStr;
    return wildcard;
}

optional<std::string> View::openFileDialog(wxWindow* parent,
                                           const DocumentType& type,
                                           const std::string& filename)
{
    wxFileDialog dialog(parent, "Open File",
                        wxEmptyString, wxEmptyString,
                        generateWildcard(type),
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (!filename.empty()) {
        dialog.SetPath(filename);
    }

    if (dialog.ShowModal() == wxID_OK && !dialog.GetPath().IsEmpty()) {
        return dialog.GetPath().ToStdString();
    }
    else {
        return optional<std::string>();
    }
}

optional<std::string> View::saveFileDialog(wxWindow* parent,
                                           const DocumentType& type,
                                           const std::string& filename)
{
    wxFileDialog dialog(parent, "Save File",
                        wxEmptyString, wxEmptyString,
                        generateWildcard(type),
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (!filename.empty()) {
        dialog.SetPath(filename);
    }

    if (dialog.ShowModal() == wxID_OK && !dialog.GetPath().IsEmpty()) {
        if (!dialog.GetFilename().Contains(".")) {
            dialog.SetFilename(dialog.GetFilename() + "." + type.extension);
        }

        return dialog.GetPath().ToStdString();
    }
    else {
        return optional<std::string>();
    }
}
