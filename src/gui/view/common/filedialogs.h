#pragma once

#include "models/common/optional.h"
#include "models/document.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {

// return value will not exist if user clicked cancel.

wxString generateWildcard(const DocumentType& type);

optional<std::string> openFileDialog(wxWindow* parent,
                                     const DocumentType& type,
                                     const Document* existing = nullptr);

optional<std::string> saveFileDialog(wxWindow* parent,
                                     const DocumentType& type,
                                     const Document* existing = nullptr);
}
}
