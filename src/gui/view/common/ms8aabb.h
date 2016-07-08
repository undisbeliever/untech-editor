#pragma once
#include "models/common/ms8aabb.h"
#include <cassert>
#include <wx/spinctrl.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {

class Ms8PointCtrl : public wxPanel {
public:
    Ms8PointCtrl(wxWindow* parent, int wxWindowID = wxID_ANY);

    ms8point GetValue() const
    {
        assert(int_ms8_t::isValid(_xPos->GetValue()));
        assert(int_ms8_t::isValid(_yPos->GetValue()));
        return ms8point(_xPos->GetValue(), _yPos->GetValue());
    }

    void SetValue(const ms8point& value)
    {
        _xPos->SetValue(value.x);
        _yPos->SetValue(value.y);
    }

private:
    wxSpinCtrl* _xPos;
    wxSpinCtrl* _yPos;
};

class Ms8RectCtrl : public wxPanel {
public:
    Ms8RectCtrl(wxWindow* parent, int wxWindowID = wxID_ANY);

    ms8rect GetValue() const
    {
        assert(int_ms8_t::isValid(_xPos->GetValue()));
        assert(int_ms8_t::isValid(_yPos->GetValue()));
        assert(_width->GetValue() >= 0 && _width->GetValue() <= 255);
        assert(_height->GetValue() >= 0 && _height->GetValue() <= 255);

        return ms8rect(_xPos->GetValue(), _yPos->GetValue(),
                       _width->GetValue(), _height->GetValue());
    }

    void SetValue(const ms8rect& value)
    {
        _xPos->SetValue(value.x);
        _yPos->SetValue(value.y);
        _width->SetValue(value.width);
        _height->SetValue(value.height);
    }

private:
    wxSpinCtrl* _xPos;
    wxSpinCtrl* _yPos;
    wxSpinCtrl* _width;
    wxSpinCtrl* _height;
};
}
}
