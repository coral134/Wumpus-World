#ifndef _ROBOT_AGENT_H
#define _ROBOT_AGENT_H

#include <map>
#include <iostream>
#include <set>
#include <cassert>
#include <list>
#include <iomanip>
#include <functional>
#include "game.h"
#include "logic_engine.h"

class RobotAgent : public Game {
public:
    RobotAgent() : Game(false, true) {}

protected:
    int wX, wY;
    std::list<Move> path;
    LogicEngine<std::pair<int, int>, CELL> logic;
    std::set<std::pair<int, int>> visited;

    void start() override;
    void update_info() override;
    Move choose_move() override;
    std::string print_cell_top(int x, int y) override;
    std::string print_cell_bottom(int x, int y) override;
    void choose_target();
    bool is_valid_cell(int x, int y);
    bool safe(int x, int y);
    bool new_safe(int x, int y);
    bool find_path_to_location(int x, int y);
    bool find_path(int startX, int startY, const std::function<bool(int, int)> &is_target);
};

#endif //_ROBOT_AGENT_H
