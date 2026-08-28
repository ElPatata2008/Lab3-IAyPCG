#include "PacmanDTController.h"

PacmanDTController::PacmanDTController(std::shared_ptr<Character> character): Controller(character) {}

PacmanDTController::~PacmanDTController() {}

Move PacmanDTController::getMove(const GameState& game) {
    
    if (character->getDirection()==PASS && game.getMaze().getPacmanStart()) {
        return RIGHT;
    }

    std::vector<int> ghostsNodes;
    std::vector<std::pair<int, int>> ghostsCoords;

    for (int i = 0; i < 4; i++) {
        ghostsNodes[i] = game.getGhostsPos(i);
        ghostsCoords[i] = game.getMaze().getNodePos(ghostsNodes[i]);
    }

    int pacmanNode = character->getPos();

    Ghost *ghost = dynamic_cast<Ghost*>(character.get());
    if (!ghost->isEdible()) {
        int minDist = 10000000;
        Move minMove;
        std::vector<Move> moves = game.getMaze().getPossibleMoves(character->getPos());

        for (Move m : moves) {
            int neighbor = game.getMaze().getNeighbour(pacmanNode, m);

            if (neighbor < 0) continue;

            auto neighborCoords = game.getMaze().getNodePos(neighbor);

        }

        return minMove;
    } else {
        int maxDist = -1;
        Move maxMove;
        std::vector<Move> moves = game.getMaze().getPossibleMoves(character->getPos());

        return maxMove;
    }

}
