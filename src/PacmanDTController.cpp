#include "PacmanDTController.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>

PacmanDTController::PacmanDTController(std::shared_ptr<Character> character): Controller(character) {}

PacmanDTController::~PacmanDTController() {}

Move PacmanDTController::getMove(const GameState& game) {
    
    if (character->getDirection()==PASS && game.getMaze().getPacmanStart()) {
        return RIGHT;
    }

    SDL_Event e;
	if( SDL_PollEvent( &e ) != 0 )
	{
		if( e.type == SDL_QUIT || 
			(e.type == SDL_KEYDOWN && 
				(e.key.keysym.sym==SDLK_ESCAPE || 
				e.key.keysym.sym==SDLK_q) ))
		{
			SDL_Quit();
			exit(0);
		}
	}

    int ghostDist = 10000000;
    int ghostNum;
    std::pair<int, int> ghostCoord;
    
    int pacmanNode = character->getPos();
    auto pacmanCoord = game.getMaze().getNodePos(pacmanNode);


    for (int i = 0; i < 4; i++) {
        int node = game.getGhostsPos(i);
        auto coords = game.getMaze().getNodePos(node);

        int sqX = std::pow(coords.first - pacmanCoord.first, 2);
        int sqY = std::pow(coords.second - pacmanCoord.second, 2);
        int sqDist = std::sqrt(sqX + sqY);

         if (sqDist < ghostDist) {
            ghostDist = sqDist;
            ghostCoord = coords;
            ghostNum = i;
         }

    }

    if (game.isGhostEdible(ghostNum)) {
        int minDist = 10000000;
        Move minMove;
        std::vector<Move> moves = game.getMaze().getPossibleMoves(pacmanNode);

        for (Move m : moves) {
            int neighbor = game.getMaze().getNeighbour(pacmanNode, m);
            if (neighbor < 0) continue;
            auto neighborCoords = game.getMaze().getNodePos(neighbor);

            int sqX = std::pow(neighborCoords.first - ghostCoord.first, 2);
            int sqY = std::pow(neighborCoords.second - ghostCoord.second, 2);
            int sqDist = std::sqrt(sqX + sqY);

            if (sqDist < minDist) {
                minDist = sqDist;
                minMove = m;
            }
        }


        return minMove;
    } else {
        int maxDist = -1;
        Move maxMove;
        std::vector<Move> moves = game.getMaze().getPossibleMoves(pacmanNode);

        for (Move m : moves) {
            int neighbor = game.getMaze().getNeighbour(pacmanNode, m);
            if (neighbor < 0) continue;
            auto neighborCoords = game.getMaze().getNodePos(neighbor);

            int sqX = std::pow(neighborCoords.first - ghostCoord.first, 2);
            int sqY = std::pow(neighborCoords.second - ghostCoord.second, 2);
            int sqDist = std::sqrt(sqX + sqY);

            if (sqDist > maxDist) {
                maxDist = sqDist;
                maxMove = m;
            }
        }

        return maxMove;
    }

}
