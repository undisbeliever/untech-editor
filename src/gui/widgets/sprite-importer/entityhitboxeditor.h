#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_ENTITYHITBOXEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_ENTITYHITBOXEDITOR_H_

#include "signals.h"
#include "models/sprite-importer/entityhitbox.h"
#include "models/sprite-importer/frame.h"
#include "models/common/string.h"
#include "gui/widgets/common/aabb.h"
#include "gui/widgets/defaults.h"

#include <cassert>
#include <memory>

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

class EntityHitboxEditor {
public:
    EntityHitboxEditor();

    void setEntityHitbox(std::shared_ptr<UnTech::SpriteImporter::EntityHitbox> entityHitbox)
    {
        _entityHitbox = entityHitbox;
        updateGuiValues();
    }

protected:
    void updateGuiValues();

    void onParameterFinishedEditing();

public:
    Gtk::Grid widget;

private:
    std::shared_ptr<UnTech::SpriteImporter::EntityHitbox> _entityHitbox;

    UrectSpinButtons _aabbSpinButtons;
    Gtk::Entry _parameterEntry;

    Gtk::Label _aabbLabel, _aabbCommaLabel, _aabbCrossLabel, _parameterLabel;

    bool _updatingValues;
};
}
}
}

#endif
