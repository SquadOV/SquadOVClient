#pragma once

#include <cassert>
#include <iostream>
#include <sstream>
#include "shared/log/log.h"

#define THROW_ERROR(x) \
    {std::ostringstream _err; \
    _err << x; \
    LOG_ERROR( _err.str() << std::endl); \
    throw std::runtime_error(_err.str());};