#pragma once

#include <random>


[[nodiscard]] inline auto& getRng()
{
    static std::random_device rd;
    static std::mt19937       rng(rd());

    return rng;
}

[[nodiscard]] inline float getRndFloat(float min, float max)
{
    return std::uniform_real_distribution<float>{min, max}(getRng());
}

template <typename T>
[[nodiscard]] inline T getRndInt(T min, T max)
{
    return std::uniform_int_distribution<T>{min, max}(getRng());
}

[[nodiscard]] inline bool getRndBool()
{
    return getRndInt<int>(0, 10) > 5;
}
