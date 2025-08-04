#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <string>
#include <string_view>

class player {
public:
    player()                         = default;
    player(player const&)            = delete;
    player(player&&)                 = delete;
    player& operator=(player const&) = delete;
    player& operator=(player&&)      = delete;
    ~player()                        = default;

    [[nodiscard]] std::string_view get_username() const { return name_; }
    std::string&                   get_username() { return name_; }

private:
    std::string name_;
};

#endif
