#pragma once
#include <vector>
#include <wx/wx.h>

namespace UnTech {
namespace View {

template <class T>
class EnumClassChoice : public wxChoice {
public:
    EnumClassChoice(wxWindow* parent, wxWindowID id)
        : wxChoice(parent, id, wxDefaultPosition, wxDefaultSize, buildNames())
    {
        _enumList.reserve(T::stringMap.size());
        for (const auto it : T::stringMap) {
            _enumList.push_back(it.second);
        }
    }

    T GetValue() const
    {
        int i = GetSelection();
        if (i >= 0 && (unsigned)i < _enumList.size()) {
            return _enumList[i];
        }
        else {
            return T();
        }
    }

    void SetValue(const T& value)
    {
        for (unsigned i = 0; i < _enumList.size(); i++) {
            if (_enumList[i] == value) {
                return SetSelection(i);
            }
        }
        return SetSelection(wxNOT_FOUND);
    }

private:
    static const wxArrayString buildNames()
    {
        wxArrayString nameList;

        if (nameList.size() == 0) {
            nameList.Alloc(T::stringMap.size());

            for (const auto it : T::stringMap) {
                nameList.Add(it.first);
            }
        }

        return nameList;
    }

private:
    std::vector<T> _enumList;
};
}
}
