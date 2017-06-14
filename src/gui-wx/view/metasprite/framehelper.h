/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <type_traits>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {

template <class FrameT>
class FrameHelper {
    using controller_type = typename std::result_of<decltype (&FrameT::Controller)(FrameT)>::type;

    FrameT* _frame;
    controller_type& _controller;

public:
    FrameHelper(FrameT* frame, controller_type& controller);

protected:
    void OnClose(wxCloseEvent& event);

    void UpdateGuiMenu();
    void UpdateGuiZoom();
    void UpdateGuiLayers();
    void UpdateGuiUndo();
    void UpdateGuiTitle();

    void OnMenuNew(wxCommandEvent&);
    void OnMenuOpen(wxCommandEvent&);
    bool SaveDocument();
    bool SaveDocumentAs();
};
}
}
}
