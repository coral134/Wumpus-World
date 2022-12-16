# Wumpus-World
A c++ Wumpus World runner

The Wumpus World is a simple, grid-based game in which the player must navigate a set of rooms to find the gold treasure, all while avoiding hazardous pits and the Evil Wumpus. At each room, only a small amount of information is provided—whether you can sense gold, the Evil Wumpus, or any pits in the four adjacent rooms. As you search for the gold based on this limited information, you must determine which rooms are safe and which are hazardous before making your move. A move consists of either walking into one of the four adjacent rooms or shooting an arrow in one of those directions if you believe the Wumpus is there. However, you only have one arrow, so it is important to be confident of the Wumpus’ whereabouts before taking a shot. Although human players can enjoy this game, it is also a good testing ground for creating AI programs to control a virtual “robot”.

In this project, the Game class loads a world from a text file and runs the game by sending sensory input to the agent and then receiving the agent's move. Agents can be created by extending the Game class and overriding the start and choose_move functions. The world size is provided as arguments of the start function, and sensory information is wrapped in a Sense object and provided as an argument of the choose_move function. Choosing a move is done by returning a Move object from choose_move.

Two agents are provided, RobotAgent and HumanAgent. RobotAgent is our implementation of a perfectly logical algorithm to play this game, and HumanAgent allows you to play the game yourself through keyboard input. A third agent, MyAgent, is also provided as a blank template to create your own agent.

The initial world is loaded in from a given text file, and can be a rectangle of any dimensions. 'E' represents an empty room, 'P' a pit, 'W' the Evil Wumpus, and 'G' the gold.

To run this code from the terminal, run the .out file with the name of the text file to load the world from and the name of the agent to use. Then, hit ENTER to continue to the next move. For example, use one of these commands to run the game:
./wumpus.out game1.txt robot
./wumpus.out game1.txt human
./wumpus.out game1.txt myagent
