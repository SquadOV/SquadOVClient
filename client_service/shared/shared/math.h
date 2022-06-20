#pragma once

namespace shared::math {

template<typename T>
T forceEven(T in) {
    return (in % 2 == 0) ? in : (in + 1);
}

// computes a * b / c.
// This weeks by assuming a = a1 + a2 where a1 is divisble by c and thus a2 = a - a1 * c (the remainder).
// Thus equivalent to:
//      = (a1 + a2) * b / c
//      = (a1 * b) / c + (a2 * b) / c
// And this way we prevent (as much as possible) super large integer multiplications and thus shouldn't get overflows.
inline int64_t i64MulDiv(int64_t a, int64_t b, int64_t c) {
    const auto remainder = a % c;
    return (a / c) * b + (remainder * b) / c;
}

}