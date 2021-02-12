#include "shared/uuid.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <sstream>

namespace shared {

std::string generateUuidv4() {
    static auto gen = boost::uuids::random_generator_mt19937();
    std::ostringstream str;
    str << gen();
    return str.str();
}

}