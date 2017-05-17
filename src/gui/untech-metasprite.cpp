/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "view/metasprite/metasprite/frame.h"
#include <wx/cmdline.h>
#include <wx/wx.h>

class MyApp : public wxApp {
public:
    virtual bool OnInit();
    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
};

static const wxCmdLineEntryDesc cmdLineDesc[] = {
    { wxCMD_LINE_SWITCH, "h", "help", "Show help text",
      wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },

    { wxCMD_LINE_PARAM, nullptr, nullptr, "files",
      wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },

    { wxCMD_LINE_NONE, nullptr, nullptr, nullptr, wxCMD_LINE_VAL_NONE, 0 }
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    SetAppDisplayName("UnTech MetaSprite Editor");
    SetAppName("untech-metasprite-gui");

    return wxApp::OnInit();
}

void MyApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc(cmdLineDesc);
    parser.SetSwitchChars("-");
}

bool MyApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    using Frame = UnTech::View::MetaSprite::MetaSprite::Frame;

    for (unsigned i = 0; i < parser.GetParamCount(); i++) {
        const std::string& filename = parser.GetParam(i).ToStdString();
        Frame::CreateOpen(filename);
    }

    if (parser.GetParamCount() == 0) {
        auto* frame = new Frame();
        frame->Show(true);
    }

    return true;
}
