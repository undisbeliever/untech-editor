#include "metaspriteapplication.h"
#include "metaspritewindow.h"
#include "version.h"
#include "gui/widgets/common/aboutdialog.h"
#include "gui/widgets/common/errormessagedialog.h"

#include <exception>
#include <iostream>

using namespace UnTech::Widgets::MetaSprite;

MetaSpriteApplication::MetaSpriteApplication()
    : Gtk::Application("net.undisbeliever.untech.metasprite",
                       Gio::APPLICATION_HANDLES_OPEN)
{
    Glib::set_application_name(UNTECH_NAME " MetaSprite");
}

Glib::RefPtr<MetaSpriteApplication> MetaSpriteApplication::create()
{
    return Glib::RefPtr<MetaSpriteApplication>(new MetaSpriteApplication());
}

void MetaSpriteApplication::on_startup()
{
    Gtk::Application::on_startup();

    add_action("new", sigc::mem_fun(*this, &MetaSpriteApplication::on_menu_new));
    add_action("open", sigc::mem_fun(*this, &MetaSpriteApplication::on_menu_open));
    add_action("about", sigc::mem_fun(*this, &MetaSpriteApplication::on_menu_about));

    _uiBuilder = Gtk::Builder::create();
    try {
        _uiBuilder->add_from_string(_uiInfo);
    }
    catch (const Glib::Error& ex) {
        std::cerr << "Building menus failed: " << ex.what();
    }

    //Get the menubar and the app menu, and add them to the application:
    auto object = _uiBuilder->get_object("menu-bar");
    auto menuBar = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
    if (!menuBar) {
        g_warning("menu-bar not found");
    }
    else {
        set_menubar(menuBar);
    }
}

void MetaSpriteApplication::on_activate()
{
    on_menu_new();
}

void MetaSpriteApplication::on_open(const type_vec_files& files, const Glib::ustring& hint)
{
    for (auto file : files) {
        if (file) {
            load_file(file->get_path());
        }
    }

    Gtk::Application::on_open(files, hint);
}

MetaSpriteWindow* MetaSpriteApplication::create_window()
{
    auto window = new MetaSpriteWindow();

    add_window(*window);

    window->signal_hide().connect(sigc::bind<Gtk::Window*>(
        sigc::mem_fun(*this, &MetaSpriteApplication::on_window_hide), window));

    window->show_all();

    return window;
}

MetaSpriteWindow* MetaSpriteApplication::empty_window()
{
    // Try and reuse current window if it has no document
    MetaSpriteWindow* window = dynamic_cast<MetaSpriteWindow*>(get_active_window());
    if (window != nullptr && window->controller().document() == nullptr) {
        return window;
    }
    else {
        return create_window();
    }
}

void MetaSpriteApplication::load_file(const std::string& filename)
{
    const std::string fullpath = File::fullPath(filename);

    // ensure the file is not already loaded
    for (auto* window : get_windows()) {
        auto* msw = dynamic_cast<MetaSpriteWindow*>(window);
        if (msw) {
            const auto* document = msw->controller().document();
            if (document && document->filename() == fullpath) {
                return;
            }
        }
    }

    auto* window = empty_window();
    window->controller().openDocument(fullpath);
}

// Delete window when hidden
// Recommended by the gtkmm examples
void MetaSpriteApplication::on_window_hide(Gtk::Window* window)
{
    delete window;
}

/*
 * MENU ACTIONS
 * ============
 */

void MetaSpriteApplication::on_menu_new()
{
    auto* window = empty_window();
    window->controller().newDocument();
}

void MetaSpriteApplication::on_menu_open()
{
    Gtk::FileChooserDialog dialog(_("Open File"),
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);

    auto* window = get_active_window();
    if (window) {
        dialog.set_transient_for(*window);
    }

    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_Open"), Gtk::RESPONSE_OK);
    dialog.set_default_response(Gtk::RESPONSE_OK);

    auto filterUtms = Gtk::FileFilter::create();
    filterUtms->set_name(_("UnTech MetaSprite File"));
    filterUtms->add_pattern("*.utms");
    dialog.add_filter(filterUtms);

    auto filterAny = Gtk::FileFilter::create();
    filterAny->set_name(_("All files"));
    filterAny->add_pattern("*");
    dialog.add_filter(filterAny);

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        load_file(dialog.get_filename());
    }
}

void MetaSpriteApplication::on_menu_about()
{
    showAboutDialog(get_active_window());
}

/*
 * UI
 * ==
 */

const Glib::ustring MetaSpriteApplication::_uiInfo
    = "<interface>"
      "  <!-- menubar -->"
      "  <menu id='menu-bar'>"
      "    <submenu>"
      "      <attribute name='label' translatable='yes'>_File</attribute>"
      "      <section>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>New</attribute>"
      "          <attribute name='action'>app.new</attribute>"
      "          <attribute name='accel'>&lt;Primary&gt;n</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>_Open</attribute>"
      "          <attribute name='action'>app.open</attribute>"
      "          <attribute name='accel'>&lt;Primary&gt;o</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>_Save</attribute>"
      "          <attribute name='action'>win.save</attribute>"
      "          <attribute name='accel'>&lt;Primary&gt;s</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>Save _As</attribute>"
      "          <attribute name='action'>win.save-as</attribute>"
      "          <attribute name='accel'>&lt;Primary&gt;&lt;Shift&gt;s</attribute>"
      "        </item>"
      "      </section>"
      "      <section>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>E_xit</attribute>"
      "          <attribute name='action'>win.exit</attribute>"
      "        </item>"
      "      </section>"
      "    </submenu>"
      "    <submenu>"
      "      <attribute name='label' translatable='yes'>_Edit</attribute>"
      "      <section>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>_Undo</attribute>"
      "          <attribute name='action'>win.undo</attribute>"
      "          <attribute name='accel'>&lt;Primary&gt;z</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>_Redo</attribute>"
      "          <attribute name='action'>win.redo</attribute>"
      "          <attribute name='accel'>&lt;Primary&gt;&lt;Shift&gt;z</attribute>"
      "        </item>"
      "      </section>"
      "      <section>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>_Add Tiles</attribute>"
      "          <attribute name='action'>win.add-tiles</attribute>"
      "          <attribute name='accel'>&lt;Primary&gt;t</attribute>"
      "        </item>"
      "      </section>"
      "      <section>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>_Create Item</attribute>"
      "          <attribute name='action'>win.create-selected</attribute>"
      "          <attribute name='accel'>&lt;Primary&gt;i</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>_Clone Selected</attribute>"
      "          <attribute name='action'>win.clone-selected</attribute>"
      "          <attribute name='accel'>&lt;Primary&gt;d</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>_Remove Selected</attribute>"
      "          <attribute name='action'>win.remove-selected</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>Move Selected _Up</attribute>"
      "          <attribute name='action'>win.move-selected-up</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>Move Selected _Down</attribute>"
      "          <attribute name='action'>win.move-selected-down</attribute>"
      "        </item>"
      "      </section>"
      "    </submenu>"
      "    <submenu>"
      "      <attribute name='label' translatable='yes'>_View</attribute>"
      "      <submenu>"
      "        <attribute name='label' translatable='yes'>_Zoom</attribute>"
      "        <item>"
      "          <attribute name='label'>_1x</attribute>"
      "          <attribute name='action'>win.set-zoom</attribute>"
      "          <attribute name='target' type='i'>1</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;1</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label'>_2x</attribute>"
      "          <attribute name='action'>win.set-zoom</attribute>"
      "          <attribute name='target' type='i'>2</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;2</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label'>_3x</attribute>"
      "          <attribute name='action'>win.set-zoom</attribute>"
      "          <attribute name='target' type='i'>3</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;3</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label'>_4x</attribute>"
      "          <attribute name='action'>win.set-zoom</attribute>"
      "          <attribute name='target' type='i'>4</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;4</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label'>_5x</attribute>"
      "          <attribute name='action'>win.set-zoom</attribute>"
      "          <attribute name='target' type='i'>5</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;5</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label'>_6x</attribute>"
      "          <attribute name='action'>win.set-zoom</attribute>"
      "          <attribute name='target' type='i'>6</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;6</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label'>_7x</attribute>"
      "          <attribute name='action'>win.set-zoom</attribute>"
      "          <attribute name='target' type='i'>7</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;7</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label'>_8x</attribute>"
      "          <attribute name='action'>win.set-zoom</attribute>"
      "          <attribute name='target' type='i'>8</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;8</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label'>_9x</attribute>"
      "          <attribute name='action'>win.set-zoom</attribute>"
      "          <attribute name='target' type='i'>9</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;9</attribute>"
      "        </item>"
      "      </submenu>"
      "      <submenu>"
      "        <attribute name='label' translatable='yes'>_Aspect Ratio</attribute>"
      "        <item>"
      "          <attribute name='label'>N_one</attribute>"
      "          <attribute name='action'>win.set-aspect-ratio</attribute>"
      "          <attribute name='target' type='i'>0</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;0</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label'>_NTSC</attribute>"
      "          <attribute name='action'>win.set-aspect-ratio</attribute>"
      "          <attribute name='target' type='i'>1</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;N</attribute>"
      "        </item>"
      "        <item>"
      "          <attribute name='label'>_PAL</attribute>"
      "          <attribute name='action'>win.set-aspect-ratio</attribute>"
      "          <attribute name='target' type='i'>2</attribute>"
      "          <attribute name='accel'>&lt;Alt&gt;P</attribute>"
      "        </item>"
      "      </submenu>"
      "      <item>"
      "        <attribute name='label' translatable='yes'>_Split View</attribute>"
      "        <attribute name='action'>win.split-view</attribute>"
      "      </item>"
      "    </submenu>"
      "    <submenu>"
      "      <attribute name='label' translatable='yes'>_Help</attribute>"
      "      <section>"
      "        <item>"
      "          <attribute name='label' translatable='yes'>_About</attribute>"
      "          <attribute name='action'>app.about</attribute>"
      "        </item>"
      "      </section>"
      "    </submenu>"
      "  </menu>"
      "</interface>";
