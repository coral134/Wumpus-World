#ifndef _MY_AGENT_H
#define _MY_AGENT_H

#include "game.h"

class MyAgent : public Game {
public:
    MyAgent() : Game(false) {}

protected:
    // Use this function to initialize values at the beginning of the game
    void start(int sizeX, int sizeY) override {

    }

    // Use this function to make your move
    // New sensory information is given in the sense object
    // Return a Move object to represent the move you have chosen to make
    Move choose_move(const Sense &sense) override {
        return walk(UP);
        // another example: return shoot(DOWN);
    }
};

#endif //_MY_AGENT_H
