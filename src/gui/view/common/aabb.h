#pragma once
#include "models/common/aabb.h"
#include <cassert>
#include <wx/spinctrl.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {

class UPointCtrl : public wxPanel {
public:
    UPointCtrl(wxWindow* parent, int wxWindowID = wxID_ANY);

    upoint GetValue() const
    {
        assert(_xPos->GetValue() >= 0);
        assert(_yPos->GetValue() >= 0);
        return upoint(_xPos->GetValue(), _yPos->GetValue());
    }

    void SetValue(const upoint& value)
    {
        _xPos->SetValue(value.x);
        _yPos->SetValue(value.y);
    }

    void SetRange(const usize& max);
    void SetRange(const usize& max, unsigned squareSize);

private:
    wxSpinCtrl* _xPos;
    wxSpinCtrl* _yPos;
};

class USizeCtrl : public wxPanel {
public:
    USizeCtrl(wxWindow* parent, int wxWindowID = wxID_ANY);

    usize GetValue() const
    {
        assert(_width->GetValue() >= 0);
        assert(_height->GetValue() >= 0);
        return usize(_width->GetValue(), _height->GetValue());
    }

    void SetValue(const usize& value)
    {
        _width->SetValue(value.width);
        _height->SetValue(value.height);
    }

    void SetRange(const usize& max);

private:
    wxSpinCtrl* _width;
    wxSpinCtrl* _height;
};

class URectCtrl : public wxPanel {
public:
    URectCtrl(wxWindow* parent, int wxWindowID = wxID_ANY);

    urect GetValue() const
    {
        assert(_xPos->GetValue() >= 0);
        assert(_yPos->GetValue() >= 0);
        assert(_width->GetValue() >= 0);
        assert(_height->GetValue() >= 0);

        return urect(_xPos->GetValue(), _yPos->GetValue(),
                     _width->GetValue(), _height->GetValue());
    }

    void SetValue(const urect& value)
    {
        _xPos->SetValue(value.x);
        _yPos->SetValue(value.y);
        _width->SetValue(value.width);
        _height->SetValue(value.height);
        UpdateRanges();
    }

    void SetMinRectSize(const usize& minSize);
    void SetRange(const usize& range);

private:
    void UpdateRanges();

private:
    usize _range, _minSize;

    wxSpinCtrl* _xPos;
    wxSpinCtrl* _yPos;
    wxSpinCtrl* _width;
    wxSpinCtrl* _height;
};
}
}
