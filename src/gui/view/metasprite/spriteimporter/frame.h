#pragma once
#include "gui/controllers/metasprite/spriteimporter.h"
#include "gui/view/common/controllerinterface.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace SpriteImporter {

namespace SI = UnTech::MetaSprite::SpriteImporter;

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
    ControllerInterface _controllerInterface;
    SI::SpriteImporterController _controller;
    wxTimer _initBugfixTimer;
};
}
}
}
}
