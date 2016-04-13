#ifndef _UNTECH_GUI_WIDGETS_COMMON_ABOUTDIALOG_H
#define _UNTECH_GUI_WIDGETS_COMMON_ABOUTDIALOG_H

#include "version.h"
#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {

inline void showAboutDialog(Gtk::Window* parent)
{
    Gtk::AboutDialog dialog;

    if (parent) {
        dialog.set_transient_for(*parent);
    }

    // ::MAYDO logo::

    dialog.set_version("Version " UNTECH_VERSION);
    dialog.set_comments(_("Part of the " UNTECH_NAME " Editor suite"));

    dialog.set_license(UNTECH_LICENSE);
    dialog.set_license_type(Gtk::LICENSE_MIT_X11);

    dialog.set_website(UNTECH_URL);
    dialog.set_website_label(UNTECH_NAME " Website");

    dialog.set_copyright(UNTECH_COPYRIGHT);

    dialog.add_credit_section(_("Coder"),
                              { "(Marcus Rowe) The UnDisbelievr" });

    dialog.add_credit_section(_("Third Party Libs"),
                              { "LodePNG http://lodev.org/lodepng/ - Copyright (c) 2005-2016 Lode Vandevenne, zlib License",
                                "GTKmm 3 http://www.gtkmm.org - Copyright 1999-2015 The gtkmm Development Team, LGPL 2.1 License" });

    dialog.run();
}
}
}
#endif
