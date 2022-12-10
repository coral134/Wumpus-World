#include <fstream>
#include <iostream>
#include <cassert>
#include <iomanip>
#include "game.h"

std::string Game::dir_to_str(DIRECTION d) {
    if(d == DIRECTION::UP) return "up";
    else if(d == DIRECTION::DOWN) return "down";
    else if(d == DIRECTION::LEFT) return "left";
    else if(d == DIRECTION::RIGHT) return "right";
    assert(false);
    return {};
}

char Game::cell_to_char(CELL c) {
    if(c == CELL::EMPTY) return 'E';
    else if(c == CELL::PIT) return 'P';
    else if(c == CELL::WUMPUS) return 'W';
    else if(c == CELL::GOLD) return 'G';
    assert(false);
    return {};
}

std::string Game::cell_to_str(CELL c) {
    if(c == CELL::EMPTY) return " ";
    else if(c == CELL::PIT) return "Pit";
    else if(c == CELL::WUMPUS) return "Wumpus";
    else if(c == CELL::GOLD) return "Gold";
    assert(false);
    return {};
}

Game::CELL Game::char_to_cell(char c) {
    if(c == 'E') return CELL::EMPTY;
    else if(c == 'P') return CELL::PIT;
    else if(c == 'W') return CELL::WUMPUS;
    else if(c == 'G') return CELL::GOLD;
    assert(false);
    return {};
}

void Game::add_direction(int &x, int &y, DIRECTION dir) {
    if(dir == DIRECTION::UP) ++y;
    else if(dir == DIRECTION::DOWN) --y;
    else if(dir == DIRECTION::RIGHT) ++x;
    else if(dir == DIRECTION::LEFT) --x;
    else
        assert(false);
}

Game::CELL Game::get(int x, int y) const {
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

bool Game::do_move(Move the_move, std::string &msg) {
    msg = "";
    just_found_gold_ = just_killed_wumpus_ = false;

    if(the_move.shoot) {
        if(used_bullet) {
            msg = "You already used your bullet!";
            return false;
        }

        int x = wX;
        int y = wY;
        add_direction(x, y, the_move.dir);
        if(get(x, y) == WUMPUS) {
            set(x, y, EMPTY);
            msg = "You killed the Wumpus!";
        } else {
            msg = "You missed your shot!";
        }

        used_bullet = true;
        return true;
    } else {
        add_direction(wX, wY, the_move.dir);
        CELL cell = get(wX, wY);

        if(found_gold && wX == 0 && wY == 0) {
            msg = "You won!!";
            return false;
        }

        if(cell != EMPTY) {
            if(cell == GOLD) {
                msg = "You found the gold, now go back home!!";
                found_gold = just_found_gold_ = true;
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

void Game::update_senses() {
    sense_gold = find(GOLD);
    sense_wumpus = find(WUMPUS);
    sense_pit = find(PIT);
}

void Game::run_game(const std::string &fileName) {
    std::ifstream stream(fileName);
    if(!stream.good()) {
        std::cerr << "Can't open " << fileName << " to read." << std::endl;
        exit(1);
    }

    std::string s;
    while(stream >> s) {
        grid.emplace_back();
        for(char c: s)
            grid[grid.size() - 1].emplace_back(char_to_cell(c));
    }

    assert(sizeY() > 0);
    assert(sizeX() > 0);

    run_game();
}

void Game::run_game() {
    wX = wY = 0;
    found_gold = just_found_gold_ = just_killed_wumpus_ = used_bullet = false;
    int move_num = 1;
    std::string msg;

    update_senses();
    start();

    while(true) {
        update_info();
        print_grid();
        std::cout << std::endl;
        print();

        std::cout << "YOU SENSE: ";
        if(sense_gold) std::cout << "glitter, ";
        if(sense_wumpus) std::cout << "stench, ";
        if(sense_pit) std::cout << "breeze, ";
        std::cout << std::endl;

        std::cin.ignore();
        Move move = choose_move();

        std::cout << std::endl << "MOVE " << move_num << ": ";
        if(move.shoot) std::cout << "shoot " << dir_to_str(move.dir);
        else std::cout << "walk " << dir_to_str(move.dir);
        std::cout << std::endl;

        bool cont = do_move(move, msg);
        ++move_num;
        update_senses();

        if(!msg.empty()) std::cout << msg << std::endl;
        if(!cont) break;
    }

    hide_world_info = false;
    print_grid();
}

void Game::print_grid() {
    int width = 7;
    std::string hbar, str;
    for(int i = 0; i < (sizeX() * (width + 1) + 1); ++i) hbar += "_";
    if(print_agent_info) hbar = hbar + "  " + hbar;
    for(int y = sizeY() - 1; y >= 0; --y) {
        std::cout << hbar << std::endl << "|";
        for(int x = 0; x < sizeX(); ++x){
            if(!hide_world_info) str = cell_to_str(get(x, y));
            else str = "";
            std::cout << std::setw(width) << str << "|";
        }
        if(print_agent_info) {
            std::cout << "  |";
            for(int x = 0; x < sizeX(); ++x)
                std::cout << std::setw(width) << print_cell_top(x, y) << "|";
        }
        std::cout << std::endl << "|";
        for(int x = 0; x < sizeX(); ++x) {
            if(wX == x && wY == y) str = "Robot";
            else str = "";
            std::cout << std::setw(width) << str << "|";
        }
        if(print_agent_info) {
            std::cout << "  |";
            for(int x = 0; x < sizeX(); ++x)
                std::cout << std::setw(width) << print_cell_bottom(x, y) << "|";
        }
        std::cout << std::endl;
    }
    std::cout << hbar << std::endl;
}