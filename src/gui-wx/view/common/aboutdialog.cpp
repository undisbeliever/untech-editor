/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "aboutdialog.h"
#include "version.h"
#include "gui-wx/view/defaults.h"
#include <wx/hyperlink.h>
#include <wx/sizer.h>
#include <wx/statline.h>

using namespace UnTech::View;

AboutDialog::AboutDialog(wxWindow* parent, const wxString& appName)
    : wxDialog(parent, wxID_ANY, "About",
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxCAPTION | wxCENTRE)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    auto* vSizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(vSizer, wxSizerFlags().Expand().DoubleBorder());

    // about
    {
        vSizer->Add(CreateLargeHText(appName),
                    wxSizerFlags().Center().Border(wxBOTTOM));

        vSizer->Add(CreateHText("Version " UNTECH_VERSION),
                    wxSizerFlags().Center().Border(wxBOTTOM));

        vSizer->Add(CreateHText("Part of the " UNTECH_NAME " Editor Suite"),
                    wxSizerFlags().Center().Border(wxBOTTOM));

        vSizer->Add(new wxHyperlinkCtrl(this, wxID_ANY, "Website", UNTECH_URL),
                    wxSizerFlags().Center());

        auto* license = new wxHyperlinkCtrl(this, wxID_ANY,
                                            "Licensed Under " UNTECH_LICENSE, UNTECH_LICENSE_URL);
        vSizer->Add(license, wxSizerFlags().Center());

        vSizer->Add(CreateHText(UNTECH_COPYRIGHT),
                    wxSizerFlags().Center().Border(wxBOTTOM));

        vSizer->Add(CreateHText("This program comes with absolutely no warranty."),
                    wxSizerFlags().Center());
    }

    // ------
    vSizer->Add(new wxStaticLine(this, wxID_ANY),
                wxSizerFlags().Expand().DoubleBorder(wxTOP | wxBOTTOM));

    // third party libs
    {
        auto* tplText = new wxStaticText(this, wxID_ANY, "Third Party Libraries:");
        tplText->SetFont(tplText->GetFont().MakeBold());
        vSizer->Add(tplText, wxSizerFlags().Border(wxBOTTOM));

        int defBorder = wxSizerFlags::GetDefaultBorder();
        auto* grid = new wxFlexGridSizer(3, 2, defBorder / 2, defBorder * 2);
        vSizer->Add(grid, wxSizerFlags().Expand().DoubleBorder(wxLEFT | wxRIGHT));

        wxSizerFlags gridFlags = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);

        grid->Add(new wxHyperlinkCtrl(this, wxID_ANY, "LodePNG", "http://lodev.org/lodepng/"),
                  gridFlags);
        grid->Add(new wxStaticText(this, wxID_ANY, "Copyright (c) 2005-2016 Lode Vandevenne, zlib License"),
                  gridFlags);

        grid->Add(new wxHyperlinkCtrl(this, wxID_ANY, "libsigc++", "http://libsigc.sourceforge.net/"),
                  gridFlags);
        grid->Add(new wxStaticText(this, wxID_ANY, "Copyright 2003-2016, The libsigc++ Development Team, LGPL License"),
                  gridFlags);

        grid->Add(new wxHyperlinkCtrl(this, wxID_ANY, "wxWidgets", "https://www.wxwidgets.org/"),
                  gridFlags);
        grid->Add(new wxStaticText(this, wxID_ANY, "Copyright (c) 1992-2016 The wxWidgets Team, wxWindows Library Licence"),
                  gridFlags);
    }

    // BUTTONS
    sizer->Add(CreateSeparatedButtonSizer(wxCLOSE),
               wxSizerFlags().Expand().DoubleBorder(wxLEFT | wxRIGHT | wxBOTTOM));

    SetAutoLayout(true);
    sizer->Fit(this);
}

inline wxStaticText* AboutDialog::CreateHText(const wxString& label)
{
    return new wxStaticText(this, wxID_ANY, label,
                            wxDefaultPosition, wxDefaultSize,
                            wxALIGN_CENTRE_HORIZONTAL);
}

inline wxStaticText* AboutDialog::CreateLargeHText(const wxString& label)
{
    auto text = new wxStaticText(this, wxID_ANY, label,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxALIGN_CENTRE_HORIZONTAL);
    text->SetFont(text->GetFont().MakeBold().MakeLarger());

    return text;
}
