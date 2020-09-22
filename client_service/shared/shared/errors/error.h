#pragma once

#include <cassert>
#include <sstream>

#define THROW_ERROR(x) \
    {std::ostringstream _err; \
    _err << x; \
    throw std::runtime_error(_err.str());};