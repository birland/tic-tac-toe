#ifndef TIMER_HPP
#define TIMER_HPP
#include <chrono>
#include <cstdint>
#include <ratio>

class timer {
public:
    timer() : m_begin_{Clock::now()} {}

    timer(timer const&)            = default;
    timer(timer&&)                 = default;
    timer& operator=(timer const&) = default;
    timer& operator=(timer&&)      = default;
    ~timer()                       = default;

    void reset() { m_begin_ = Clock::now(); }

    [[nodiscard]] double elapsed_seconds() const {
        return std::chrono::duration_cast<Second>(Clock::now() - m_begin_)
            .count();
    }
    [[nodiscard]] std::uint64_t elapsed_milliseconds() const {
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now() - m_begin_
            )
                .count()
        );
    }

    [[nodiscard]] std::uint64_t elapsed_nanoseconds() const {
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                Clock::now() - m_begin_
            )
                .count()
        );
    }

private:
    using Second = std::chrono::duration<unsigned, std::ratio<1>>;
    using Clock  = std::chrono::steady_clock;
    std::chrono::time_point<Clock> m_begin_;
};

#endif
