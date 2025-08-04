#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP
#include <array>
#include <cstdint>
#include <fmt/base.h>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

struct state {
    struct menu {};
    struct play {};
    struct exit {};
};

class state_machine {
public:
    using states = std::variant<state::menu, state::play, state::exit>;

    state_machine()                                = default;
    state_machine(state_machine const&)            = delete;
    state_machine(state_machine&&)                 = delete;
    state_machine& operator=(state_machine const&) = delete;
    state_machine& operator=(state_machine&&)      = delete;
    ~state_machine()                               = default;

    template <typename... Ts>
    struct overloaded : Ts... { // NOLINT(fuchsia-*)
        using Ts::operator()...;
    };

    // Set state, i.e. state::menu
    template <typename Tp>
    void set(Tp new_state) {

        var_ = new_state;
    }

    template <typename Tp>
    state_machine& operator=(Tp new_state) {
        previous_var_ = var_;
        set(new_state);
        return *this;
    }

    // Return variant of the states to std::visit on them
    states const& get_variant() { return var_; }
    states const& get_previous_variant() { return previous_var_; }

    constexpr std::string_view str_v() {
        return std::visit(
            overloaded{
                [](state::menu) {
                    return str_states_[std::to_underlying(idx_e::MENU)];
                },
                [](state::play) {
                    return str_states_[std::to_underlying(idx_e::PLAY)];
                },
                [](state::exit) {
                    return str_states_[std::to_underlying(idx_e::EXIT)];
                },
            },
            var_
        );
    }

    constexpr std::string_view str_v(auto&& val) {
        using Tp = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<Tp, state::menu>) {
            return str_states_[std::to_underlying(idx_e::MENU)];
        } else if (std::is_same_v<Tp, state::play>) {
            return str_states_[std::to_underlying(idx_e::PLAY)];
        } else if (std::is_same_v<Tp, state::exit>) {
            return str_states_[std::to_underlying(idx_e::EXIT)];
        }
        throw std::logic_error("Unexpected type");
    }

    bool compare(auto&& val) {
        using Tp = std::decay_t<decltype(val)>;
        return std::visit(
            overloaded{
                [](state::menu) { return std::is_same_v<state::menu, Tp>; },
                [](state::play) { return std::is_same_v<state::play, Tp>; },
                [](state::exit) { return std::is_same_v<state::exit, Tp>; },
            },
            var_
        );
    }

    template <typename Tp>
    bool operator==(Tp const& other_state) {
        return compare(other_state);
    }

    template <typename Tp>
    bool operator!=(Tp const& other_state) {
        return !(*this == other_state);
    }

    enum class idx_e : std::uint8_t { MENU, PLAY, EXIT, SIZE };

private:
    states var_;
    // When selected 'NO' button to prevent exit
    // we are using saved previous variant of the states
    states previous_var_;
    //////////////////////////////////////////
    static constexpr std::array str_states_{"MENU", "PLAY", "EXIT"};
    static_assert(str_states_.size() == std::to_underlying(idx_e::SIZE));
};

#endif
