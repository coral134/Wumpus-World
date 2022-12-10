#include "robot_agent.h"

void RobotAgent::start() {
    wX = wY = 0;
    logic = LogicEngine<std::pair<int, int>, CELL>({CELL::EMPTY, CELL::PIT, CELL::WUMPUS, CELL::GOLD});
    path = std::list<Move>();
}

void RobotAgent::update_info() {
    if(just_found_gold()) {
        logic.set_known({wX, wY}, EMPTY);
        path = std::list<Move>();
    }

    if(!path.empty()) return;

    visited.insert({wX, wY});
    logic.constrain_all_of({{wX, wY}}, CELL::EMPTY);

    std::vector<std::pair<int, int>> locs;
    if(is_valid_cell(wX, wY + 1)) locs.emplace_back(wX, wY + 1);
    if(is_valid_cell(wX, wY - 1)) locs.emplace_back(wX, wY - 1);
    if(is_valid_cell(wX + 1, wY)) locs.emplace_back(wX + 1, wY);
    if(is_valid_cell(wX - 1, wY)) locs.emplace_back(wX - 1, wY);

    if(stench()) logic.constrain_one_of(locs, CELL::WUMPUS);
    else logic.constrain_none_of(locs, CELL::WUMPUS);
    if(glitter()) logic.constrain_one_of(locs, CELL::GOLD);
    else logic.constrain_none_of(locs, CELL::GOLD);
    if(breeze()) logic.constrain_at_least_one_of(locs, CELL::PIT);
    else logic.constrain_none_of(locs, CELL::PIT);
    logic.constrain_one_of(CELL::GOLD);
    logic.constrain_one_of(CELL::WUMPUS);
    logic.deduce();

    choose_target();
}

Game::Move RobotAgent::choose_move() {
    Move move = path.front();
    if(move.shoot) {
        int x = wX;
        int y = wY;
        add_direction(x, y, move.dir);
        logic.set_known({x, y}, EMPTY);
    } else {
        add_direction(wX, wY, move.dir);
    }

    path.pop_front();
    return move;
}

std::string RobotAgent::print_cell_top(int x, int y) {
    auto res = logic.most_likely({x, y});
    if(wX == x && wY == y) return "Robot";
    if(res.first.size() == 1)
        return cell_to_str(res.first.back());
    std::string str;
    for(auto i: res.first) {
        str += cell_to_char(i);
        if(i != res.first.size() - 1) str += "/";
    }
    return str;
}

std::string RobotAgent::print_cell_bottom(int x, int y) {
    auto res = logic.most_likely({x, y});
    if(wX == x && wY == y) return "";
    int l = (int) (res.second * 100);
    if(l == 100) return " ";
    std::string str = std::to_string(l);
    str += "%";
    return str;
}

void RobotAgent::choose_target() {
    path = std::list<Move>();
    std::pair<int, int> loc;

    if(just_found_gold()) { // navigate back
        find_path_to_location(0, 0);
        return;
    }

    if(logic.find_by_state(GOLD, loc)) { // navigate to gold
        find_path_to_location(loc.first, loc.second);
        return;
    }

    if(logic.find_by_state(WUMPUS, loc)) { // hunt the wumpus
        find_path_to_location(loc.first, loc.second);
        path.insert(std::prev(path.end()), shoot(path.back().dir));
        return;
    }

    // find path to a new, safe cell
    if(find_path(wX, wY, [this](int x, int y) { return new_safe(x, y); }))
        return;

    // No great options, but pick the best one
    if(logic.highest_prob({GOLD, EMPTY}, loc)) {
        find_path_to_location(loc.first, loc.second);
        return;
    }

    // No possible safe options
    std::cerr << "This game is rigged!" << std::endl;
    exit(1);
}

bool RobotAgent::is_valid_cell(int x, int y) {
    return !(x < 0 || y < 0 || x >= sizeX() || y >= sizeY());
}

bool RobotAgent::safe(int x, int y) {
    return logic.is_true({x, y}, CELL::EMPTY);
}

bool RobotAgent::new_safe(int x, int y) {
    return (visited.find({x, y}) == visited.end()) && safe(x, y);
}

bool RobotAgent::find_path_to_location(int x, int y) {
    return find_path(wX, wY, [x, y](int x_, int y_) { return x_ == x && y_ == y; });
}

bool RobotAgent::find_path(int startX, int startY, const std::function<bool(int, int)> &is_target) {
    std::set<std::pair<int, int>> used_cells;
    std::vector<std::pair<int, int>> new_cells;
    std::vector<std::list<Move>> paths;
    std::vector<DIRECTION> directions = {DIRECTION::UP, DIRECTION::DOWN, DIRECTION::RIGHT, DIRECTION::LEFT};

    used_cells.insert({startX, startY});
    new_cells.emplace_back(startX, startY);
    paths.emplace_back();

    int times = 0;
    while(!paths.empty()) {
        std::vector<std::pair<int, int>> temp_new_cells;
        std::vector<std::list<Move>> temp_paths;

        for(int i = 0; i < (int)paths.size(); ++i) {
            for(auto dir: directions) {
                int x2 = new_cells[i].first;
                int y2 = new_cells[i].second;
                add_direction(x2, y2, dir);
                if(is_target(x2, y2)) { // found end
                    path = paths[i];
                    path.push_back(walk(dir));
                    return true;
                }
                if(safe(x2, y2) && (used_cells.find({x2, y2}) == used_cells.end())) {
                    // new valid step
                    temp_new_cells.emplace_back(x2, y2);
                    temp_paths.push_back(paths[i]);
                    temp_paths.back().push_back(walk(dir));
                    used_cells.insert({x2, y2});
                }
            }
        }

        paths = temp_paths;
        new_cells = temp_new_cells;
        ++times;
    }

    return false;
}