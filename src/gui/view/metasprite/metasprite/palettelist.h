#pragma once
#include "gui/controllers/metasprite/metasprite.h"
#include "gui/view/defaults.h"
#include <cassert>
#include <wx/listctrl.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite::MetaSprite;

class PaletteListCtrl : public wxListCtrl {
public:
    const unsigned N_COLORS = Snes::Palette4bpp::N_COLORS;
    const unsigned COLOR_SIZE = 16;
    const unsigned COLOR_SPACING = 0;

    const unsigned IMAGE_HEIGHT = COLOR_SIZE;
    const unsigned IMAGE_WIDTH = COLOR_SIZE * N_COLORS + COLOR_SPACING * (N_COLORS - 1);

public:
    PaletteListCtrl(wxWindow* parent, wxWindowID id,
                    MS::PaletteController& controller);

private:
    void RedrawImageList();

    virtual wxString OnGetItemText(long item, long column) const override;
    virtual int OnGetItemColumnImage(long item, long column) const override;

private:
    MS::PaletteController& _controller;
};
}
}
}
}
