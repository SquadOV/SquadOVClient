#pragma once

#include <cassert>
#include <sstream>

#define THROW_ERROR(x) \
    std::ostringstream err; \
    err << x; \
    throw std::runtime_error(err.str());