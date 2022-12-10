#include "robot_agent.h"
#include "human_agent.h"

int main(int argc, char *argv[]) {
    if(argc < 3) {
        std::cerr << "Please provide a game file and agent name!" << std::endl;
        std::cerr << "Example inputs:" << std::endl;
        std::cerr << "./wumpus.out game1.txt robot" << std::endl;
        std::cerr << "./wumpus.out game1.txt human" << std::endl;
        exit(1);
    }

    if(std::string(argv[2]) == "robot"){
        RobotAgent agent;
        agent.run_game(argv[1]);
    } else if(std::string(argv[2]) == "human"){
        HumanAgent agent;
        agent.run_game(argv[1]);
    }
}