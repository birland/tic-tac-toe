#ifndef __BOARD_HPP__
#define __BOARD_HPP__

class board {
public:
    board()                        = default;
    board(board const&)            = default;
    board(board&&)                 = default;
    board& operator=(board const&) = default;
    board& operator=(board&&)      = default;
    ~board()                       = default;

private:
};

#endif
