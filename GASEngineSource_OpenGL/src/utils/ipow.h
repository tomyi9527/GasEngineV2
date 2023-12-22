#pragma once

// for pow
template <typename T>
constexpr T ipow(T num, unsigned int pow) {
    return (pow >= sizeof(unsigned int) * 8) ? 0 : pow == 0 ? 1 : num * ipow(num, pow - 1);
}