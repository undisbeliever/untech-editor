#include "spriteimporterapplication.h"
#include "spriteimporterwindow.h"
#include "../common/errormessagedialog.h"
#include "../common/aboutdialog.h"
#include "version.h"

#include <iostream>
#include <exception>

using namespace UnTech::Widgets::SpriteImporter;

SpriteImporterApplication::SpriteImporterApplication()
    : Gtk::Application("net.undisbeliever.untech.sprite-importer",
                       Gio::APPLICATION_HANDLES_OPEN)
{
    Glib::set_application_name(UNTECH_NAME " Sprite Importer");
}

Glib::RefPtr<SpriteImporterApplication> SpriteImporterApplication::create()
{
    return Glib::RefPtr<SpriteImporterApplication>(new SpriteImporterApplication());
}

void SpriteImporterApplication::on_startup()
{
    Gtk::Application::on_startup();

    add_action("new", sigc::mem_fun(*this, &SpriteImporterApplication::on_menu_new));
    add_action("open", sigc::mem_fun(*this, &SpriteImporterApplication::on_menu_open));
    add_action("about", sigc::mem_fun(*this, &SpriteImporterApplication::on_menu_about));

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

void SpriteImporterApplication::on_activate()
{
    create_window(std::make_unique<Document>());
}

void SpriteImporterApplication::on_open(const type_vec_files& files, const Glib::ustring& hint)
{
    for (auto file : files) {
        if (file) {
            load_file(file->get_path());
        }
    }

    Gtk::Application::on_open(files, hint);
}

void SpriteImporterApplication::create_window(std::unique_ptr<Document> document)
{
    if (document) {
        auto window = new SpriteImporterWindow();

        // ::DEBUG create a document::
        window->setDocument(std::move(document));

        add_window(*window);

        window->signal_hide().connect(sigc::bind<Gtk::Window*>(
            sigc::mem_fun(*this, &SpriteImporterApplication::on_window_hide), window));

        window->show_all();
    }
}

void SpriteImporterApplication::load_file(const std::string& filename)
{
    // ensure the file is not already loaded
    for (auto* window : get_windows()) {
        auto* siw = dynamic_cast<SpriteImporterWindow*>(window);
        if (siw) {
            if (siw->document()->filename() == filename) {
                return;
            }
        }
    }

    std::unique_ptr<Document> document;

    try {
        document = std::make_unique<Document>(filename);
    }
    catch (const std::exception& ex) {
        showErrorMessage(get_active_window(), "Unable to open file", ex);

        return;
    }

    create_window(std::move(document));
}

// Delete window when hidden
// Recommended by the gtkmm examples
void SpriteImporterApplication::on_window_hide(Gtk::Window* window)
{
    delete window;
}

/*
 * MENU ACTIONS
 * ============
 */

void SpriteImporterApplication::on_menu_new()
{
    create_window(std::make_unique<Document>());
}

void SpriteImporterApplication::on_menu_open()
{
    Gtk::FileChooserDialog dialog(*get_active_window(),
                                  _("Open File"),
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);

    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_Open"), Gtk::RESPONSE_OK);
    dialog.set_default_response(Gtk::RESPONSE_OK);

    auto filterUtsi = Gtk::FileFilter::create();
    filterUtsi->set_name(_("UnTech Sprite Importer File"));
    filterUtsi->add_pattern("*.utsi");
    dialog.add_filter(filterUtsi);

    auto filterAny = Gtk::FileFilter::create();
    filterAny->set_name(_("All files"));
    filterAny->add_pattern("*");
    dialog.add_filter(filterAny);

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        load_file(dialog.get_filename());
    }
}

void SpriteImporterApplication::on_menu_about()
{
    showAboutDialog(get_active_window());
}

/*
 * UI
 * ==
 */

const Glib::ustring SpriteImporterApplication::_uiInfo
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
