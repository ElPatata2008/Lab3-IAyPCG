#include "FSMGhost.h"
#include <iostream>

int FSMGhostCS = 0;

bool FSMGhostGuardDecided = false;
bool FSMGhostGoGuard = false;
// int FSMGhostGuardIndex = 0;
std::pair<int, int> FSMGhostGuardTarget = {-1, -1};

FSMGhost::FSMGhost(std::shared_ptr<Character> character):
	Controller(character),
	e(rand()),
	uniform_dist(0,3),
	fsm(std::make_shared<FSMGhostStateMachine>(character)){
}
FSMGhost::~FSMGhost() {}
Move FSMGhost::getMove(const GameState& game){ return fsm->update(game); }

#pragma region Transitions

FSMGhostFrightenedTransition::FSMGhostFrightenedTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character){ }
bool FSMGhostFrightenedTransition::isValid(const GameState& gs) {
	Ghost *ghost = dynamic_cast<Ghost*>(_character.get());
	if (ghost->isEdible() == true){ return true; }
	return false;
}
std::shared_ptr<FSMState> FSMGhostFrightenedTransition::getNextState() { return _next; }

FSMGhostUnfrightTransition::FSMGhostUnfrightTransition(std::shared_ptr<FSMState> next1, std::shared_ptr<FSMState> next2, std::shared_ptr<Character> character):
 _next1(next1), _next2(next2), _character(character) {}
bool FSMGhostUnfrightTransition::isValid(const GameState& gs) {
	Ghost *ghost = dynamic_cast<Ghost*>(_character.get());
	if (ghost->isEdible() == false) { return true; }
	return false;
}
std::shared_ptr<FSMState> FSMGhostUnfrightTransition::getNextState() { 
	switch (FSMGhostCS) 
	{
		case 1: return _next1; break;
		case 2: return _next2; break;
	
		default: return _next1; break; //Safety Net
	}
}

FSMGhostChaseTransition::FSMGhostChaseTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool FSMGhostChaseTransition::isValid(const GameState& gs) {
    std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - _start;
    double duration = 12;
    if (diff.count() > duration) {
        return true;
    }
	return false;
}
std::shared_ptr<FSMState> FSMGhostChaseTransition::getNextState() { return _next; }

FSMGhostGuardTransition::FSMGhostGuardTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool FSMGhostGuardTransition::isValid(const GameState& gs) {
	if (gs.getMaze().getPowerPillPositions().empty()) return false;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end - _start;
	if (diff.count() > 20.0) {
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> FSMGhostGuardTransition::getNextState() { return _next; }

FSMGhostUnguardTransition::FSMGhostUnguardTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool FSMGhostUnguardTransition::isValid(const GameState& gs) {
	auto pps = gs.getMaze().getPowerPillPositions();
	bool stillThere = std::find(pps.begin(), pps.end(), FSMGhostGuardTarget) != pps.end();
	if (pps.empty() || !stillThere) return true;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end - _start;
	if (diff.count() > 12.0) { return true; }
	return false;
}
std::shared_ptr<FSMState> FSMGhostUnguardTransition::getNextState() { return _next; }

#pragma endregion

#pragma region States

FSMGhostChaseState::FSMGhostChaseState(std::shared_ptr<Character> _character):FSMState(_character){ }
void FSMGhostChaseState::onEnter(const GameState& ){ 
	// std::cout << "FSMGhost Chasing..." << std::endl;

	// std::dynamic_pointer_cast<Ghost>(character)->revert(); 

	FSMGhostCS = 1;

	for (auto& t : transitions) {
		auto st = std::dynamic_pointer_cast<FSMGhostGuardTransition>(t);
		if (st) { st->_start = std::chrono::high_resolution_clock::now(); }
	}
}
Move FSMGhostChaseState::onUpdate(const GameState& game) {
	
	std::vector<Move> moves;
	const auto pacmanCoord=game.getMaze().getNodePos(game.getPacmanPos());
	const auto myPos=character->getPos();
	//const auto myCoord=game.getMaze().getNodePos(myPos);

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float min= moves[0] == PASS ? 10000000 : euclid2(
		game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[0])),
			pacmanCoord);
	int minI=0;
	for(unsigned int i=1;i<moves.size();i++){
		auto dist=euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[i])),
			pacmanCoord);
		if(dist<min){
			min=dist;
			minI=i;
		}
	}
	return moves[minI];
}
FSMGhostChaseState::~FSMGhostChaseState(){}

FSMGhostGuardState::FSMGhostGuardState(std::shared_ptr<Character> _character):FSMState(_character){ }
void FSMGhostGuardState::onEnter(const GameState& gs){ 
	// std::cout << "FSMGhost Chasing..." << std::endl;
	auto pps = gs.getMaze().getPowerPillPositions();

	if (pps.empty()) { target = std::make_pair(108, 4); }
	else { 
		target = pps[rand() % pps.size()]; 
		FSMGhostGuardTarget = target;
	}

	FSMGhostCS = 2;

	for (auto& t : transitions) {
		auto stc = std::dynamic_pointer_cast<FSMGhostChaseTransition>(t);
		if (stc) { stc->_start = std::chrono::high_resolution_clock::now(); }
		auto stug = std::dynamic_pointer_cast<FSMGhostUnguardTransition>(t);
		if (stug) { stug->_start = std::chrono::high_resolution_clock::now(); }
	}
}
Move FSMGhostGuardState::onUpdate(const GameState& game) {
	std::vector<Move> moves;
	const auto myPos=character->getPos();
	// std::cerr << game.getMaze().getNodePos(myPos).first << " " << game.getMaze().getNodePos(myPos).second << "\n";

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float min= moves[0] == PASS ? 10000000 : euclid2(
		game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[0])),
			target);
	int minI=0;
	for(unsigned int i=1;i<moves.size();i++){
		auto dist=euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[i])),
			target);
		if(dist<min){
			min=dist;
			minI=i;
		}
	}
	return moves[minI];
}
FSMGhostGuardState::~FSMGhostGuardState(){}

FSMGhostFrightenedState::FSMGhostFrightenedState(std::shared_ptr<Character> _character) : FSMState(_character) {}
void FSMGhostFrightenedState::onEnter(const GameState& ) { 
	// std::cout << "FSMGhost Escaping..." << std::endl; 
}
Move FSMGhostFrightenedState::onUpdate(const GameState& game) {
	std::vector<Move> moves;
	const auto pacmanCoord=game.getMaze().getNodePos(game.getPacmanPos());
	const auto myPos=character->getPos();
	//const auto myCoord=game.getMaze().getNodePos(myPos);

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float max= moves[0] == PASS ? -1 :
		euclid2(
		game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[0])),
			pacmanCoord);
	int maxI=0;
	for(unsigned int i=1;i<moves.size();i++){
		auto dist=euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[i])),
			pacmanCoord);
		if(dist>max){
			max=dist;
			maxI=i;
		}
	}
	return moves[maxI];
}
FSMGhostFrightenedState::~FSMGhostFrightenedState(){}

#pragma endregion

FSMGhostStateMachine::FSMGhostStateMachine(std::shared_ptr<Character> _character) : FiniteStateMachine(_character) {
	auto chaseState = std::make_shared<FSMGhostChaseState>(character);
	auto guardState = std::make_shared<FSMGhostGuardState>(character);
	auto frightenedState = std::make_shared<FSMGhostFrightenedState>(character);

	frightenedState->addTransition(std::make_shared<FSMGhostUnfrightTransition>(chaseState, guardState, character));

	guardState->addTransition(std::make_shared<FSMGhostFrightenedTransition>(frightenedState, character));
	guardState->addTransition(std::make_shared<FSMGhostChaseTransition>(chaseState, character));
	guardState->addTransition(std::make_shared<FSMGhostUnguardTransition>(chaseState, character));

	chaseState->addTransition(std::make_shared<FSMGhostFrightenedTransition>(frightenedState, character));
	chaseState->addTransition(std::make_shared<FSMGhostGuardTransition>(guardState, character));
	
	states.push_back(chaseState);
	states.push_back(frightenedState);
	states.push_back(guardState);

	initialState = guardState;
	activeState = initialState;
}
Move FSMGhostStateMachine::update(const GameState& gs) {
	auto t = activeState->getActiveTransition(gs);
	if (t != nullptr) {
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}
FSMGhostStateMachine::~FSMGhostStateMachine() {}
