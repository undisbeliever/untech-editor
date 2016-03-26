#ifndef _UNTECH_GUI_WIDGETS_DOCUMENT_UNDODOCUMENT_H_
#define _UNTECH_GUI_WIDGETS_DOCUMENT_UNDODOCUMENT_H_

#include "gui/undo/undodocument.h"
#include "models/sprite-importer/document.h"

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

class Document
    : public UnTech::SpriteImporter::SpriteImporterDocument,
      public Undo::UndoDocument {
public:
    Document() = default;

    Document(const std::string& filename)
        : UnTech::SpriteImporter::SpriteImporterDocument(filename)
        , Undo::UndoDocument()
    {
    }

    virtual ~Document() = default;
};
}
}
}
#endif
