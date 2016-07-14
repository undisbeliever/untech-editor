#pragma once
#include "gui/controllers/metasprite.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class Window : public wxFrame {
public:
    Window();

    MS::MetaSpriteController& Controller() { return _controller; }

    static void CreateOpen(const std::string& filename);

protected:
    void OnMenuNew(wxCommandEvent&);
    void OnMenuOpen(wxCommandEvent&);
    void OnMenuSave(wxCommandEvent&);
    void OnMenuSaveAs(wxCommandEvent&);

private:
    MS::MetaSpriteController _controller;
};
}
}
}
