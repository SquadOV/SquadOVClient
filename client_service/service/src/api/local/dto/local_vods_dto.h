#pragma once

#include <oatpp/core/data/mapping/type/Object.hpp>
#include <oatpp/core/macro/codegen.hpp>

#include OATPP_CODEGEN_BEGIN(DTO)

namespace service::api::local::dto {

class LocalVodsDto: public oatpp::DTO {
    DTO_INIT(LocalVodsDto, DTO);
    DTO_FIELD(Vector<String>, data);
};

}

#include OATPP_CODEGEN_END(DTO)