/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/optional.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {

struct DocumentType {
    const wxString name;
    const wxString extension;
};

// return value will not exist if user clicked cancel.

wxString generateWildcard(const DocumentType& type);

optional<std::string> openFileDialog(wxWindow* parent,
                                     const DocumentType& type,
                                     const std::string& filename = "");

optional<std::string> saveFileDialog(wxWindow* parent,
                                     const DocumentType& type,
                                     const std::string& filename = "");
}
}
