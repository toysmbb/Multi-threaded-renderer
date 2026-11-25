#pragma once
#include <cstdint>
#include <random>

class Random
{
public:
    static void Init() { s_engine.seed(std::random_device{}()); }
    static void Seed(std::uint32_t seed) { s_engine.seed(seed); }

    static float Float() { return s_distribution(s_engine); }

private:
    inline static std::mt19937 s_engine{ 0x9E3779B9u };
    inline static std::uniform_real_distribution<float> s_distribution{ 0.0f, 1.0f };
};
