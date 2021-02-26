#pragma once

namespace shared::math {

template<typename T>
T forceEven(T in) {
    return (in % 2 == 0) ? in : (in + 1);
}

}