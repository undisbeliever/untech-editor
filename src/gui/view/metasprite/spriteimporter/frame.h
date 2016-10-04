#pragma once
#include "gui/controllers/sprite-importer.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class Frame : public wxFrame {
public:
    const static wxString WINDOW_NAME;

public:
    Frame();

    SI::SpriteImporterController& Controller() { return _controller; }

    static void CreateOpen(const std::string& filename);

protected:
    void UpdateGuiMenu();
    void UpdateGuiZoom();
    void UpdateGuiLayers();
    void UpdateGuiUndo();
    void UpdateGuiTitle();

    void OnMenuNew(wxCommandEvent&);
    void OnMenuOpen(wxCommandEvent&);

    bool SaveDocument();
    bool SaveDocumentAs();

    void OnClose(wxCloseEvent&);

private:
    SI::SpriteImporterController _controller;
    wxTimer _initBugfixTimer;
};
}
}
}
