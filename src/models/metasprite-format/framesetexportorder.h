#ifndef _UNTECH_MODELS_METASPRITEFORMAT_FRAMESETEXPORTORDER_H
#define _UNTECH_MODELS_METASPRITEFORMAT_FRAMESETEXPORTORDER_H

#include "framesetexportorderserializer.h"
#include "models/common/namedlist.h"
#include "models/common/orderedlist.h"
#include "models/common/orderednamedlist.h"
#include "models/document.h"
#include <string>

namespace UnTech {
namespace MetaSpriteFormat {

namespace FrameSetExportOrder {

class ExportOrderDocument;
class ExportOrder;
class ExportName;
class AlternativeName;

class ExportOrder {
public:
    typedef NamedList<ExportOrderDocument, ExportOrder> list_t;

public:
    ExportOrder() = delete;
    ExportOrder(const ExportOrder&) = delete;

    ExportOrder(ExportOrderDocument& document)
        : _document(document)
        , _name()
        , _stillFrames(*this)
        , _animations(*this)
    {
    }

    ~ExportOrder() = default;

    inline ExportOrderDocument& document() const { return _document; }

    const std::string& name() const { return _name; }

    void setName(const std::string& newName)
    {
        if (_name != newName && isNameValid(newName)) {
            _name = newName;
        }
    }

    inline auto& stillFrames() { return _stillFrames; }
    inline const auto& stillFrames() const { return _stillFrames; }

    inline auto& animations() { return _animations; }
    inline const auto& animations() const { return _animations; }

private:
    ExportOrderDocument& _document;

    std::string _name;
    OrderedNamedList<ExportOrder, ExportName> _stillFrames;
    OrderedNamedList<ExportOrder, ExportName> _animations;
};

class ExportName {
public:
    typedef OrderedNamedList<ExportOrder, ExportName> list_t;

public:
    ExportName() = delete;
    ExportName(const ExportOrder&) = delete;

    ExportName(ExportOrder& exportOrder)
        : _exportOrder(exportOrder)
        , _alternativeNames(*this)
    {
    }

    ExportName(const ExportName& ns, ExportOrder& exportOrder)
        : _exportOrder(exportOrder)
        , _alternativeNames(*this)
    {
        for (const auto& d : ns._alternativeNames) {
            _alternativeNames.clone(d);
        }
    }

    ExportOrder& exportOrder() const { return _exportOrder; }
    inline ExportOrderDocument& document() const { return _exportOrder.document(); }

    inline auto& alternativeNames() { return _alternativeNames; }
    inline const auto& alternativeNames() const { return _alternativeNames; }

private:
    ExportOrder& _exportOrder;

    OrderedList<ExportName, AlternativeName> _alternativeNames;
};

class AlternativeName {
public:
    typedef OrderedList<ExportName, AlternativeName> list_t;

public:
    AlternativeName() = delete;
    AlternativeName(const AlternativeName&) = delete;

    AlternativeName(ExportName& exportName)
        : _exportName(exportName)
        , _name()
        , _hFlip(false)
        , _vFlip(false)
    {
    }

    AlternativeName(const AlternativeName& alt, ExportName& exportName)
        : _exportName(exportName)
        , _name(alt._name)
        , _hFlip(alt._hFlip)
        , _vFlip(alt._vFlip)
    {
    }

    ExportName& exportName() const { return _exportName; }
    inline ExportOrderDocument& document() const { return _exportName.document(); }

    inline const std::string& name() const { return _name; }
    inline bool hFlip() const { return _hFlip; }
    inline bool vFlip() const { return _vFlip; }

    void setName(const std::string& newName)
    {
        if (_name != newName && isNameValid(newName)) {
            _name = newName;
        }
    }

    void setHFlip(bool hFlip) { _hFlip = hFlip; }
    void setVFlip(bool vFlip) { _vFlip = vFlip; }

private:
    ExportName& _exportName;

    std::string _name;
    bool _hFlip;
    bool _vFlip;
};

class ExportOrderDocument : public ::UnTech::Document {
public:
    ExportOrderDocument();
    explicit ExportOrderDocument(const std::string& filename);

    virtual ~ExportOrderDocument() = default;

    inline auto& exportOrder() { return _exportOrder; }
    inline const auto& exportOrder() const { return _exportOrder; }

    virtual void writeDataFile(const std::string& filename) override;

    static std::shared_ptr<const ExportOrderDocument> loadReadOnly(const std::string& filename);

private:
    ExportOrder _exportOrder;
};
}
}
}

#endif
