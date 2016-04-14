#ifndef _UNTECH_GUI_WIDGETS_DOCUMENT_UNDODOCUMENT_H_
#define _UNTECH_GUI_WIDGETS_DOCUMENT_UNDODOCUMENT_H_

#include "gui/undo/undodocument.h"
#include "models/metasprite/document.h"

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

class Document
    : public UnTech::MetaSprite::MetaSpriteDocument,
      public Undo::UndoDocument {
public:
    Document() = default;

    Document(const std::string& filename)
        : UnTech::MetaSprite::MetaSpriteDocument(filename)
        , Undo::UndoDocument()
    {
    }

    virtual ~Document() = default;
};
}
}
}
#endif
