#pragma once
#include <vector>
#include <wx/wx.h>

namespace UnTech {
namespace View {

template <class T>
class EnumClassChoice : public wxChoice {
public:
    static const std::vector<T> enumList;
    static const wxArrayString nameList;

public:
    EnumClassChoice(wxWindow* parent, wxWindowID id)
        : wxChoice(parent, id, wxDefaultPosition, wxDefaultSize, nameList)
    {
    }

    T GetValue() const
    {
        int i = GetSelection();
        if (i >= 0 && (unsigned)i < enumList.size()) {
            return enumList[i];
        }
        else {
            return T();
        }
    }

    void SetValue(const T& value)
    {
        for (unsigned i = 0; i < enumList.size(); i++) {
            if (enumList[i] == value) {
                return SetSelection(i);
            }
        }
        return SetSelection(wxNOT_FOUND);
    }

private:
    static const std::vector<T> __buildEnums()
    {
        std::vector<T> ret;
        ret.reserve(T::stringMap.size());

        for (const auto it : T::stringMap) {
            ret.push_back(it.second);
        }

        return ret;
    }
    static const wxArrayString __buildNames()
    {
        wxArrayString ret;
        ret.Alloc(T::stringMap.size());

        for (const auto it : T::stringMap) {
            ret.Add(it.first);
        }

        return ret;
    }
};

template <typename T>
const std::vector<T> EnumClassChoice<T>::enumList = EnumClassChoice<T>::__buildEnums();

template <typename T>
const wxArrayString EnumClassChoice<T>::nameList = EnumClassChoice<T>::__buildNames();
}
}
