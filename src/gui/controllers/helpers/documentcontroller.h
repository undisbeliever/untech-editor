#pragma once

#include "gui/controllers/basecontroller.h"
#include "models/common/orderedlist.h"
#include <memory>

namespace UnTech {
namespace Controller {

template <class DocumentT>
class DocumentController : public BaseController {
public:
    DocumentController(std::unique_ptr<ControllerInterface>);
    DocumentController(const DocumentController&) = delete;
    ~DocumentController() = default;

    const DocumentT* document() const { return _document.get(); }

    auto signal_documentChanged() { return _signal_documentChanged; }

    void newDocument();
    void openDocument(const std::string& filename);
    bool saveDocument();
    bool saveDocumentAs(const std::string& filename);

protected:
    DocumentT* document_editable() { return _document.get(); }

    void setDocument(std::unique_ptr<DocumentT>);

private:
    std::unique_ptr<DocumentT> _document;

    sigc::signal<void> _signal_documentChanged;
};
}
}
