#pragma once
#include "gui/controllers/metasprite/metasprite.h"
#include "gui/view/common/controllerinterface.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite::MetaSprite;

class GraphicsPanel;
class Sidebar;

class Frame : public wxFrame {
public:
    const static wxString WINDOW_NAME;

public:
    Frame();

    MS::MetaSpriteController& Controller() { return _controller; }

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
    MS::MetaSpriteController _controller;
    wxTimer _initBugfixTimer;

    GraphicsPanel* _graphics;
    Sidebar* _sidebar;
};
}
}
}
}
