#include "documentcontroller.h"

namespace UnTech {
namespace Controller {

template <class DocumentT>
DocumentController<DocumentT>::DocumentController(std::unique_ptr<ControllerInterface> interface)
    : BaseController(std::move(interface))
    , _document(nullptr)
{
}

template <class DocumentT>
void DocumentController<DocumentT>::setDocument(std::unique_ptr<DocumentT> document)
{
    std::unique_ptr<DocumentT> oldDocument = std::move(_document);

    undoStack().clear();

    // Emits document Changed twice to ensure all references
    // are removed by View before activating new document.
    if (oldDocument != nullptr) {
        _signal_documentChanged.emit();
    }

    _document = std::move(document);
    _signal_documentChanged.emit();
}

template <class DocumentT>
void DocumentController<DocumentT>::newDocument()
{
    setDocument(std::make_unique<DocumentT>());
    undoStack().markDirty();
}

template <class DocumentT>
bool DocumentController<DocumentT>::openDocument(const std::string& filename)
{
    std::unique_ptr<DocumentT> document;

    try {
        document = std::make_unique<DocumentT>(filename);
    }
    catch (const std::exception& ex) {
        showError("Unable to load document", ex);
    }

    setDocument(std::move(document));

    return _document != nullptr;
}

template <class DocumentT>
bool DocumentController<DocumentT>::saveDocument()
{
    if (_document == nullptr) {
        return true;
    }

    if (_document->filename().empty()) {
        return false;
    }

    try {
        _document->save();
        undoStack().markClean();
        return true;
    }
    catch (const std::exception& ex) {
        showError("Unable to save document", ex);
        return false;
    }
}

template <class DocumentT>
bool DocumentController<DocumentT>::saveDocumentAs(const std::string& filename)
{
    if (_document == nullptr) {
        return true;
    }

    if (filename.empty()) {
        return false;
    }

    try {
        _document->saveFile(filename);
        undoStack().markClean();
        return true;
    }
    catch (const std::exception& ex) {
        showError("Unable to save document", ex);
        return false;
    }
}
}
}
