#pragma once

#include <cassert>
#include <iostream>
#include <sstream>

#define THROW_ERROR(x) \
    {std::ostringstream _err; \
    _err << x; \
    std::cerr << "ERROR: " << _err.str() << std::endl; \
    throw std::runtime_error(_err.str());};