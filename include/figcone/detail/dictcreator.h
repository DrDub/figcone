#pragma once
#include "dict.h"
#include "iconfigreader.h"
#include "validator.h"
#include "external/sfun/contract.h"
#include "external/sfun/type_traits.h"
#include <figcone/nameformat.h>
#include <memory>

namespace figcone::detail {

template<typename TMap>
class DictCreator {
public:
    DictCreator(ConfigReaderPtr cfgReader, std::string dictName, TMap& dictMap)
        : cfgReader_{cfgReader}
        , dictName_{(sfun_precondition(!dictName.empty()), std::move(dictName))}
        , dict_{std::make_unique<Dict<TMap>>(dictName_, dictMap)}
        , dictMap_{dictMap}
    {
        static_assert(
                sfun::is_associative_container_v<sfun::remove_optional_t<TMap>>,
                "Dictionary field must be an associative container or an associative container placed in "
                "std::optional");
        static_assert(
                std::is_same_v<typename sfun::remove_optional_t<TMap>::key_type, std::string>,
                "Dictionary associative container's key type must be std::string");
    }

    DictCreator& operator()(TMap defaultValue = {})
    {
        dict_->markValueIsSet();
        defaultValue_ = std::move(defaultValue);
        return *this;
    }

    operator TMap()
    {
        if (cfgReader_)
            cfgReader_->addNode(dictName_, std::move(dict_));
        return defaultValue_;
    }

    DictCreator& checkedWith(std::function<void(const TMap&)> validatingFunc)
    {
        if (cfgReader_)
            cfgReader_->addValidator(std::make_unique<Validator<TMap>>(*dict_, dictMap_, std::move(validatingFunc)));
        return *this;
    }

    template<typename TValidator, typename... TArgs>
    DictCreator& checkedWith(TArgs&&... args)
    {
        if (cfgReader_)
            cfgReader_->addValidator(
                    std::make_unique<Validator<TMap>>(*dict_, dictMap_, TValidator{std::forward<TArgs>(args)...}));
        return *this;
    }

private:
    ConfigReaderPtr cfgReader_;
    std::string dictName_;
    std::unique_ptr<Dict<TMap>> dict_;
    TMap& dictMap_;
    TMap defaultValue_;
};

} //namespace figcone::detail