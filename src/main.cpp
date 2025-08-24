#include <cstdio> // for stderr
#include <cstdlib>
#include <exception>
#include <fmt/base.h>
#include "engine.hpp"

int main() {
    try {
        engine eng;
        eng.run();
    } catch (std::exception const& e) {
        fmt::println(stderr, "{}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        fmt::println(stderr, "Unknown exception");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
