#include <fstream>
#include <iostream>
#include <cassert>
#include <iomanip>
#include "game.h"

void Game::run_game(const std::string &fileName) {
    std::ifstream stream(fileName);
    if(!stream.good()) {
        std::cerr << "Can't open " << fileName << " to read." << std::endl;
        exit(1);
    }

    std::string s;
    while(stream >> s) {
        grid.emplace_back();
        for(char c: s) grid[grid.size() - 1].emplace_back(to_cell(c));
    }

    assert(sizeY() > 0);
    assert(sizeX() > 0);

    run_game();
}

void Game::run_game() {
    wX = wY = 0;
    found_gold = used_bullet = false;
    int move_num = 1;
    std::string msg;
    Sense sense;
    start(sizeX(), sizeY());

    while(true) {
        print_grid();
        if(!msg.empty()) std::cout << "MESSAGE: " << msg << std::endl;
        update_senses(sense);
        print_senses(sense);
        std::cin.ignore();
        Move move = choose_move(sense);
        print_move(move, move_num);
        sense = Sense();
        if(!do_move(move, sense, msg)) break;
        ++move_num;
    }

    hide_world_info = false;
    print_grid();
    if(!msg.empty()) std::cout << "MESSAGE: " << msg << std::endl;
}

bool Game::do_move(const Move &move, Sense &sense, std::string &msg) {
    msg = "";

    if(move.shoot) {
        if(used_bullet) {
            msg = "You already used your bullet!";
            return false;
        }

        int x = wX;
        int y = wY;
        add_direction(x, y, move.dir);

        if(get(x, y) == WUMPUS) {
            set(x, y, EMPTY);
            msg = "You killed the Wumpus!";
            sense.just_killed_wumpus = true;
        } else {
            msg = "You missed your shot!";
        }

        used_bullet = true;
        return true;
    } else {
        add_direction(wX, wY, move.dir);
        CELL cell = get(wX, wY);

        if(found_gold && wX == 0 && wY == 0) {
            msg = "You won!!";
            return false;
        }

        if(cell != EMPTY) {
            if(cell == GOLD) {
                msg = "You found the gold, now go back home!!";
                found_gold = sense.just_found_gold = true;
                set(wX, wY, EMPTY);
                return true;
            }
            if(cell == WUMPUS) msg = "You ran into the Evil Wumpus!";
            else if(cell == PIT) msg = "You fell into a pit!";
            else if(cell == WALL) msg = "You left the playing area!";
            return false;
        }

        return true;
    }
}

void Game::update_senses(Sense &sense) {
    sense.glitter = find(GOLD);
    sense.stench = find(WUMPUS);
    sense.breeze = find(PIT);
}

CELL Game::get(int x, int y) const {
    if(x < 0 || y < 0 || x >= sizeX() || y >= sizeY())
        return WALL;
    return grid[sizeY() - 1 - y][x];
}

void Game::set(int x, int y, CELL c) {
    assert(!(x < 0 || y < 0 || x >= sizeX() || y >= sizeY()));
    grid[sizeY() - 1 - y][x] = c;
}

bool Game::find(CELL c) const {
    if(get(wX + 1, wY) == c) return true;
    if(get(wX - 1, wY) == c) return true;
    if(get(wX, wY + 1) == c) return true;
    if(get(wX, wY - 1) == c) return true;
    return false;
}

void Game::print_senses(const Sense &sense) {
    std::cout << "YOU SENSE: ";
    bool is_first = true;
    if(sense.glitter) {
        std::cout << "glitter";
        is_first = false;
    }
    if(sense.stench) {
        if(!is_first) std::cout << ", ";
        std::cout << "stench";
        is_first = false;
    }
    if(sense.breeze) {
        if(!is_first) std::cout << ", ";
        std::cout << "breeze";
        is_first = false;
    }
    if(is_first) std::cout << "nothing";
    std::cout << std::endl;
}

void Game::print_move(const Move &move, int move_num) {
    std::cout << std::endl << "MOVE " << move_num << ": ";
    if(move.shoot) std::cout << "shoot " << to_str(move.dir);
    else std::cout << "walk " << to_str(move.dir);
    std::cout << std::endl;
}

void Game::print_grid() {
    int width = 7;
    std::string hbar, str;
    for(int i = 0; i < (sizeX() * (width + 1) + 1); ++i) hbar += "_";

    for(int y = sizeY() - 1; y >= 0; --y) {
        std::cout << hbar << std::endl << "|";
        for(int x = 0; x < sizeX(); ++x){
            if(!hide_world_info) str = to_str(get(x, y));
            else str = "";
            std::cout << std::setw(width) << str << "|";
        }
        std::cout << std::endl << "|";
        for(int x = 0; x < sizeX(); ++x) {
            if(wX == x && wY == y) str = "Robot";
            else str = "";
            std::cout << std::setw(width) << str << "|";
        }
        std::cout << std::endl;
    }
    std::cout << hbar << std::endl;
}

void Game::add_direction(int &x, int &y, DIRECTION dir) {
    if(dir == DIRECTION::UP) ++y;
    else if(dir == DIRECTION::DOWN) --y;
    else if(dir == DIRECTION::RIGHT) ++x;
    else if(dir == DIRECTION::LEFT) --x;
    else assert(false);
}

std::string Game::to_str(DIRECTION d) {
    if(d == DIRECTION::UP) return "up";
    else if(d == DIRECTION::DOWN) return "down";
    else if(d == DIRECTION::LEFT) return "left";
    else if(d == DIRECTION::RIGHT) return "right";
    assert(false);
    return {};
}

std::string Game::to_str(CELL c) {
    if(c == CELL::EMPTY) return " ";
    else if(c == CELL::PIT) return "Pit";
    else if(c == CELL::WUMPUS) return "Wumpus";
    else if(c == CELL::GOLD) return "Gold";
    assert(false);
    return {};
}

CELL Game::to_cell(char c) {
    if(c == 'E') return CELL::EMPTY;
    else if(c == 'P') return CELL::PIT;
    else if(c == 'W') return CELL::WUMPUS;
    else if(c == 'G') return CELL::GOLD;
    assert(false);
    return {};
}