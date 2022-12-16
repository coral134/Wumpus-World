#ifndef _GAME_H
#define _GAME_H

#include <vector>
#include <string>

// Enums to represent a direction and the contents of a square in the world
enum DIRECTION { UP, RIGHT, DOWN, LEFT };
enum CELL { EMPTY, PIT, WUMPUS, GOLD, WALL };

// Represents a move by the agent
class Move {
public:
    Move(bool shoot_, DIRECTION dir_) : shoot(shoot_), dir(dir_) {}
    bool shoot; DIRECTION dir;
};

// Represents the agent's current senses
class Sense {
public:
    Sense() : glitter(false), breeze(false), stench(false), just_found_gold(false), just_killed_wumpus(false) {}
    bool glitter, breeze, stench, just_found_gold, just_killed_wumpus;
};

class Game {
public:
    // Loads in a game file and runs the game
    void run_game(const std::string &fileName);

protected:
    Game(bool hide_world_info_ = false) : hide_world_info(hide_world_info_) {}

    // Override this function to initialize values at the beginning of the game
    virtual void start(int sizeX, int sizeY) {}
    // Override this function to choose your move
    virtual Move choose_move(const Sense &sense) { return walk(DIRECTION::DOWN); };

    // Use these functions to make your move
    static Move walk(DIRECTION dir) { return {false, dir}; }
    static Move shoot(DIRECTION dir) { return {true, dir}; }

private:
    std::vector<std::vector<CELL>> grid;
    int wX, wY;
    bool used_bullet, found_gold, hide_world_info;

    void run_game();
    bool do_move(const Move &move, Sense &sense, std::string &msg);
    void update_senses(Sense &sense);

    CELL get(int x, int y) const;
    void set(int x, int y, CELL c);
    bool find(CELL c) const;
    int sizeX() const { return (int)grid[0].size(); };
    int sizeY() const { return (int)grid.size(); };

    static void print_senses(const Sense &sense);
    static void print_move(const Move &move, int move_num);
    void print_grid();

    static void add_direction(int &x, int &y, DIRECTION dir);
    static CELL to_cell(char c);
    static std::string to_str(CELL c);
    static std::string to_str(DIRECTION d);
};

#endif //_GAME_H
