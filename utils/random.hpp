#ifndef RANDOM_HPP
#define RANDOM_HPP
#include <chrono>
#include <random>

namespace psdrng {
    inline std::mt19937 generate() {
        std::random_device rdv{};
        std::seed_seq      seed_s{
            static_cast<std::seed_seq::result_type>(
                std::chrono::steady_clock::now().time_since_epoch().count()
            ),
            rdv(),
            rdv(),
            rdv(),
            rdv(),
            rdv(),
            rdv(),
            rdv(),
            rdv()
        };

        return std::mt19937{seed_s};
    }

    template <typename T>
    inline T get(T min, T max) {
        std::mt19937 mt{generate()};
        return std::uniform_int_distribution<T>{min, max}(mt);
    }
} // namespace psdrng

#endif
