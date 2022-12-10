#ifndef _GAME_H
#define _GAME_H

#include <vector>
#include <string>

class Game {
public:
    // Loads in a game file and runs the game
    void run_game(const std::string &fileName);

protected:
    // Enums to represent a direction and the contents of a square in the world
    enum DIRECTION { UP, RIGHT, DOWN, LEFT };
    enum CELL { EMPTY, PIT, WUMPUS, GOLD, WALL };

    // Represents a move by the agent
    class Move {
    public:
        Move(bool shoot_, DIRECTION dir_) : shoot(shoot_), dir(dir_) {}
        bool shoot; DIRECTION dir;
    };

    Game(bool hide_world_info_ = false, bool print_agent_info_ = false) : print_agent_info(print_agent_info_),
        hide_world_info(hide_world_info_) {}

    virtual void start() {} // Use to initialize values at the beginning of the game
    virtual void update_info() {} // Use to update your knowledge about the world
    virtual Move choose_move() { return walk(DIRECTION::DOWN); }; // Use this function to make your next move
    virtual void print() {} // Use this function if you want to print info each turn or debug
    virtual std::string print_cell_top(int, int) { return {}; }
    virtual std::string print_cell_bottom(int, int) { return {}; }

    // Use these functions to get information about the world
    bool breeze() const { return sense_pit; }
    bool stench() const { return sense_wumpus; }
    bool glitter() const { return sense_gold; }
    bool just_found_gold() const { return just_found_gold_; }
    bool just_killed_wumpus() const { return just_killed_wumpus_; }
    int sizeX() const { return (int)grid[0].size(); };
    int sizeY() const { return (int)grid.size(); };

    // Use these functions to make your move
    static Move walk(DIRECTION dir) { return Move(false, dir); }
    static Move shoot(DIRECTION dir) { return Move(true, dir); }

    // Some helpful functions
    static void add_direction(int &x, int &y, DIRECTION dir);
    static char cell_to_char(CELL c);
    static CELL char_to_cell(char c);
    static std::string cell_to_str(CELL c);
    static std::string dir_to_str(DIRECTION d);

private:
    std::vector<std::vector<CELL>> grid;
    int wX, wY;
    bool used_bullet, found_gold;
    bool sense_gold, sense_pit, sense_wumpus, just_found_gold_, just_killed_wumpus_;
    bool print_agent_info, hide_world_info;

    CELL get(int x, int y) const;
    void set(int x, int y, CELL c);
    bool find(CELL c) const;
    bool do_move(Move the_move, std::string &msg);
    void update_senses();
    void run_game();
    void print_grid();
};

#endif //_GAME_H
