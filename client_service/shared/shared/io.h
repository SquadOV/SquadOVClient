#pragma once

#include <ostream>
#include <optional>

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& p) {
    if (p) {
        os << p.value();
    } else {
        os << "None";
    }
    return os;
}