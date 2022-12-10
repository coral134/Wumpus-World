#ifndef _HUMAN_AGENT_H
#define _HUMAN_AGENT_H

#include <map>
#include <iostream>
#include <set>
#include <cassert>
#include <list>
#include <iomanip>
#include <functional>
#include "game.h"
#include "logic_engine.h"

class HumanAgent : public Game {
public:
    HumanAgent() : Game(true, false) {}

protected:
    DIRECTION choose_shoot() {
        while(true) {
            std::cout << "Where do you shoot: w (up), s (down), a (left), d (right)" << std::endl;
            std::string str;
            std::cin >> str;
            if(str == "w") return UP;
            else if(str == "s") return DOWN;
            else if(str == "a") return LEFT;
            else if(str == "d") return RIGHT;
        }
    }

    Move choose_move() override {
        while(true) {
            std::cout << "Choose your move: w (up), s (down), a (left), d (right), x (shoot)" << std::endl;
            std::string str;
            std::cin >> str;
            if(str == "w") return walk(UP);
            else if(str == "s") return walk(DOWN);
            else if(str == "a") return walk(LEFT);
            else if(str == "d") return walk(RIGHT);
            else if(str == "x") return shoot(choose_shoot());
        }
    }
};

#endif //_HUMAN_AGENT_H
