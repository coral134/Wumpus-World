#ifndef _ROBOT_AGENT_H
#define _ROBOT_AGENT_H

#include <set>
#include <list>
#include <functional>
#include "game.h"
#include "logic_engine.h"

class RobotAgent : public Game {
public:
    RobotAgent() : Game(false) {}

protected:
    std::list<Move> path;
    LogicEngine<std::pair<int, int>, CELL> logic;
    std::set<std::pair<int, int>> visited;
    Sense sense;
    int sX, sY, wX, wY;

    void start(int sizeX, int sizeY) override;
    Move choose_move(const Sense &sense_) override;

    void update_info();
    Move follow_path();
    void choose_target();

    bool find_path_to_location(int x, int y);
    bool find_path(int startX, int startY, const std::function<bool(int, int)> &is_target);

    static void add_direction(int &x, int &y, DIRECTION dir);
    bool is_valid_cell(int x, int y) const;
    bool safe(int x, int y);
    bool new_safe(int x, int y);
};

#endif //_ROBOT_AGENT_H
