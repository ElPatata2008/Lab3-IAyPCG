#include "PacmanController.h"
#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>

PacmanController::PacmanController(std::shared_ptr<Character> character): 
	Controller(character), 
	e(rand()),
	uniform_dist(0,3),
	fsm(std::make_shared<PacmanStateMachine>(character)) {
}
PacmanController::~PacmanController() { }
Move PacmanController::getMove(const GameState& game){
	//para cerrar la ventana
	SDL_Event e;
	if( SDL_PollEvent( &e ) != 0 )
	{
		if( e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && (e.key.keysym.sym==SDLK_ESCAPE || e.key.keysym.sym==SDLK_q) ))
		{
			SDL_Quit();
			exit(0);
		}
	}

	return fsm->update(game);	
}

#pragma region Transitions

LookForPowerTransition::LookForPowerTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character): _next(next), _character(character) {}
bool LookForPowerTransition::isValid(const GameState& gs) {
	bool powerPillNearby = false;
	bool ghostNearby = false;

	if (gs.getMaze().getPowerPillPositions().size() == 0) return false;
	else {
		for (int i = 0; i < gs.getMaze().getPowerPillPositions().size(); i++) {
			if (euclid2(gs.getMaze().getNodePos(_character->getPos()), 
				gs.getMaze().getPowerPillPositions()[i]) < 400) {
				powerPillNearby = true;
				break;
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		if (euclid2(gs.getMaze().getNodePos(_character->getPos()), 
			gs.getMaze().getNodePos(gs.getGhostsPos(i))) < 400 && !gs.isGhostEdible(i)) {
			ghostNearby = true;
			break;

		}
	}

	return ghostNearby && powerPillNearby ? true : false;
}
std::shared_ptr<FSMState> LookForPowerTransition::getNextState() { return _next; }

PowerOutOfRangeTransition::PowerOutOfRangeTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character): _next(next), _character(character) {}
bool PowerOutOfRangeTransition::isValid(const GameState& gs) {
	if (gs.getMaze().getPowerPillPositions().size() == 0) return false;
	else {
		for (int i = 0; i < gs.getMaze().getPowerPillPositions().size(); i++) {
			if (euclid2(gs.getMaze().getNodePos(_character->getPos()), 
				gs.getMaze().getPowerPillPositions()[i]) > 400) {
				return true;
			}
		}
	}

	return false;
}
std::shared_ptr<FSMState> PowerOutOfRangeTransition::getNextState() { return _next; }

LookForPillsTransition::LookForPillsTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character): _next(next), _character(character) {}
bool LookForPillsTransition::isValid(const GameState& gs) {
	// bool ghostNearby = true;
	for (int i = 0; i < 4; i++) {
		auto dist = euclid2(gs.getMaze().getNodePos(_character->getPos()), gs.getMaze().getNodePos(gs.getGhostsPos(i)));
		if (dist < 400 && !gs.isGhostEdible(i)) return false;
	}
	return true;
	// return !ghostNearby ? true : false;
}
std::shared_ptr<FSMState> LookForPillsTransition::getNextState() { return _next; }

AttackGhostsTransition::AttackGhostsTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character): _next(next),_character(character) {}
bool AttackGhostsTransition::isValid(const GameState& gs) {
	for (int i = 0; i < 4; i++) if (gs.isGhostEdible(i)) return true;
	return false;
}
std::shared_ptr<FSMState> AttackGhostsTransition::getNextState() { return _next; }

PowerDownTransition::PowerDownTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character): _next(next),_character(character) {}
bool PowerDownTransition::isValid(const GameState& gs) {
	for (int i = 0; i < 4; i++) if (gs.isGhostEdible(i)) return false;
	return true;
}
std::shared_ptr<FSMState> PowerDownTransition::getNextState() { return _next; }

RunFromGhostsTransition::RunFromGhostsTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character): _next(next),_character(character) {}
bool RunFromGhostsTransition::isValid(const GameState& gs) {
	for (int i = 0; i < 4; i++) {
		auto dist = euclid2(gs.getMaze().getNodePos(_character->getPos()), gs.getMaze().getNodePos(gs.getGhostsPos(i)));
		if (dist < 400 && !gs.isGhostEdible(i)) return true;
	}
	return false;
}
std::shared_ptr<FSMState> RunFromGhostsTransition::getNextState() { return _next; }

#pragma endregion

#pragma region States

SearchPowerState::SearchPowerState(std::shared_ptr<Character> _character) : FSMState(_character) { }
void SearchPowerState::onEnter(const GameState& ) { }
Move SearchPowerState::onUpdate(const GameState& game) { 
	const auto myCoords = game.getMaze().getNodePos(character->getPos());
	std::pair<int, int> ppCoords;

	int minDistPP = 10000000;
	if (game.getMaze().getPowerPillPositions().size() > 0) {
		for (int i = 0; i < game.getMaze().getPowerPillPositions().size(); i++) {
			auto node = game.getMaze().getPowerPillPositions()[i];

			float dist = euclid2(myCoords, node);
			if (dist < minDistPP) {
				minDistPP = dist;
				ppCoords = node;
			}
		}
	}

	std::vector<Move> moves;
	if (character->getDirection() == PASS) {
		moves = game.getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}
	Move minMove = moves[0];
	float minDist = (minMove == PASS) ? 1000000 : euclid2(myCoords, ppCoords);

	for (unsigned int i = 1; i < moves.size(); i++) {
		if (moves[i] == PASS) continue;
		auto min = euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(character->getPos(), moves[i])),
			ppCoords
		);
		if (min < minDist) {
			minDist = min;
			minMove = moves[i];
		}
	}

	return minMove; 
}
SearchPowerState::~SearchPowerState(){}

ScavengeState::ScavengeState(std::shared_ptr<Character> _character) : FSMState(_character) { }
void ScavengeState::onEnter(const GameState& ) {}
Move ScavengeState::onUpdate(const GameState& game) { 

	const auto myCoords = game.getMaze().getNodePos(character->getPos());
	std::pair<int, int> pillCoords;

	std::vector<std::pair<float, std::pair<int, int>>> dists;
	if (game.getMaze().getPillPositions().size() > 0) {
		for (int i = 0; i < game.getMaze().getPillPositions().size(); i++) {
			std::pair<int, int> pill = game.getMaze().getPillPositions()[i];
			float dist = euclid2(myCoords, pill);
			dists.push_back({dist, pill});
		}
	}
	std::sort(dists.begin(), dists.end());
	int N = std::min(3, static_cast<int>(dists.size()));
	pillCoords = dists[rand() % N].second;

	if (dists.empty()) {
		int minDistPill = 10000000;
		if (game.getMaze().getPillPositions().size() > 0) {
			for (int i = 0; i < game.getMaze().getPillPositions().size(); i++) {
				auto node = game.getMaze().getPillPositions()[i];
				
				float dist = euclid2(myCoords, node);
				if (dist < minDistPill) {
					minDistPill = dist;
					pillCoords = node;
				}
			}
		}
	}

	std::vector<Move> moves;
	if (character->getDirection() == PASS) {
		moves = game.getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}
	// Move minMove = moves[0];
	std::vector<Move> bestMoves;
	bestMoves.push_back(moves[0]);
	int minDist = (moves[0] == PASS) ? 1000000 : euclid2(pillCoords, myCoords);

	for (unsigned int i = 1; i < moves.size(); i++) {
		if (moves[i] == PASS) continue;
		int min = euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(character->getPos(), moves[i])),
			pillCoords
		);
		if (min < minDist) {
			minDist = min;
			bestMoves.clear();
			bestMoves.push_back(moves[i]);
			// bestMoves = [moves[i]];
			// minMove = moves[i];
		}
		else if (std::abs(min - minDist) < 0.001) {
			bestMoves.push_back(moves[i]);
		}
	}

	// minMove = 

	return bestMoves[rand() % bestMoves.size()];
}
ScavengeState::~ScavengeState(){}

EscapeState::EscapeState(std::shared_ptr<Character> _character) : FSMState(_character) { }
void EscapeState::onEnter(const GameState& ) {}
Move EscapeState::onUpdate(const GameState& game) { 
	int ghostDist = 10000000;
    std::pair<int, int> ghostCoord;
    auto myCoord = game.getMaze().getNodePos(character->getPos());

    for (int i = 0; i < 4; i++) {
        auto coords = game.getMaze().getNodePos(game.getGhostsPos(i));

        float dist = euclid2(myCoord, coords);
         if (dist < ghostDist) {
            ghostDist = dist;
            ghostCoord = coords;
         }
    }

	std::vector<Move> moves;
	if (character->getDirection() == PASS) {
		moves = game.getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}
	Move maxMove = moves[0];
	float maxDist = (maxMove == PASS) ? -1 : euclid2(ghostCoord, myCoord);

	for (unsigned int i = 1; i < moves.size(); i++) {
		if (moves[i] == PASS) continue;
		auto dist = euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(character->getPos(), moves[i])),
			ghostCoord
		);
		if (dist > maxDist) {
			maxDist = dist;
			maxMove = moves[i];
		}
	}

	return maxMove; 
}
EscapeState::~EscapeState(){}

AttackState::AttackState(std::shared_ptr<Character> _character) : FSMState(_character) { }
void AttackState::onEnter(const GameState& ) {}
Move AttackState::onUpdate(const GameState& game) { 
	int ghostDist = 10000000;
    std::pair<int, int> ghostCoord;
    auto myCoord = game.getMaze().getNodePos(character->getPos());

    for (int i = 0; i < 4; i++) {
        auto coords = game.getMaze().getNodePos(game.getGhostsPos(i));

        float dist = euclid2(myCoord, coords);
         if (dist < ghostDist && game.isGhostEdible(i)) {
            ghostDist = dist;
            ghostCoord = coords;
         }
    }

	std::vector<Move> moves;
	if (character->getDirection() == PASS) {
		moves = game.getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}
	Move minMove = moves[0];
	float minDist = (minMove == PASS) ? 1000000 : euclid2(game.getMaze().getNodePos(game.getMaze().getNeighbour(character->getPos(), minMove)), ghostCoord);

	// std::vector<Move> totalMoves;
	for (unsigned int i = 1; i < moves.size(); i++) {
		if (moves[i] == PASS) continue;
		auto dist = euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(character->getPos(), moves[i])),
			ghostCoord
		);

		if (dist < minDist) {
			minDist = dist;
			minMove = moves[i];
			// totalMoves.push_back(moves[i]);
		}
	}

	// minMove = totalMoves[rand() % totalMoves.size()];

	return minMove;
}
AttackState::~AttackState(){}

#pragma endregion
 
PacmanStateMachine::PacmanStateMachine(std::shared_ptr<Character> _character) : FiniteStateMachine(_character) {
	auto searchPowerState = std::make_shared<SearchPowerState>(character);
	auto scavengeState = std::make_shared<ScavengeState>(character);
	auto escapeState = std::make_shared<EscapeState>(character);
	auto attackState = std::make_shared<AttackState>(character);

	scavengeState->addTransition(std::make_shared<AttackGhostsTransition>(attackState, character));
	scavengeState->addTransition(std::make_shared<RunFromGhostsTransition>(escapeState, character));

	escapeState->addTransition(std::make_shared<LookForPowerTransition>(searchPowerState, character));
	escapeState->addTransition(std::make_shared<LookForPillsTransition>(scavengeState, character));

	searchPowerState->addTransition(std::make_shared<AttackGhostsTransition>(attackState, character));
	searchPowerState->addTransition(std::make_shared<PowerOutOfRangeTransition>(scavengeState, character));

	attackState->addTransition(std::make_shared<PowerDownTransition>(scavengeState, character));

	states.push_back(searchPowerState);
	states.push_back(scavengeState);
	states.push_back(escapeState);
	states.push_back(attackState);

	initialState = scavengeState;
	activeState = initialState;

}
Move PacmanStateMachine::update(const GameState& gs) {
	auto t = activeState->getActiveTransition(gs);
	if (t != nullptr) {
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}
PacmanStateMachine::~PacmanStateMachine(){}