#pragma once
#include "gui/controllers/metasprite/metasprite.h"
#include <wx/spinctrl.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace MetaSprite {

class AddTilesDialog : public wxDialog {
public:
    AddTilesDialog(wxWindow* parent,
                   unsigned nSmall, unsigned nLarge);

    unsigned GetSmallToAdd() const
    {
        assert(_addSmall->GetValue() >= 0);
        return _addSmall->GetValue();
    }

    unsigned GetLargeToAdd() const
    {
        assert(_addLarge->GetValue() >= 0);
        return _addLarge->GetValue();
    }

private:
    void UpdateGui();

private:
    unsigned _nSmall;
    unsigned _nLarge;

    wxSpinCtrl* _addSmall;
    wxSpinCtrl* _addLarge;

    wxStaticText* _smallTotal;
    wxStaticText* _largeTotal;
};
}
}
}
}
