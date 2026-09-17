#include "BlinkyController.h"
#include <iostream>

bool cruiseElroy = false;
int blinkyCS = 1;
int totalPills = 0; 

bool blinkyGuardDecided = false;
bool blinkyGoGuard = false;
// int blinkyGuardIndex = 0;
std::pair<int, int> blinkyGuardTarget = {-1, -1};

BlinkyController::BlinkyController(std::shared_ptr<Character> character):
	Controller(character),
	e(rand()),
	uniform_dist(0,3),
	fsm(std::make_shared<BlinkyStateMachine>(character)){
}
BlinkyController::~BlinkyController() {}
Move BlinkyController::getMove(const GameState& game){ 
	Maze mazeCopy = game.getMaze();
	int current = mazeCopy.getPillPositions().size() + mazeCopy.getPowerPillPositions().size();
	if (current > totalPills) {
		totalPills = current;
		cruiseElroy = false;
	}
	
	return fsm->update(game); 
}

#pragma region Transitions

BlinkyFrightenedTransition::BlinkyFrightenedTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character){ }
bool BlinkyFrightenedTransition::isValid(const GameState& gs) {
	Ghost *ghost = dynamic_cast<Ghost*>(_character.get());
	if (ghost->isEdible() == true){ return true; }
	return false;
}
std::shared_ptr<FSMState> BlinkyFrightenedTransition::getNextState() { return _next; }

BlinkyUnfrightTransition::BlinkyUnfrightTransition(std::shared_ptr<FSMState> next1, std::shared_ptr<FSMState> next2, std::shared_ptr<FSMState> next3, std::shared_ptr<Character> character):
 _next1(next1), _next2(next2), _next3(next3), _character(character) {}
bool BlinkyUnfrightTransition::isValid(const GameState& gs) {
	Ghost *ghost = dynamic_cast<Ghost*>(_character.get());
	if (gs.getMaze().getPillPositions().size() + gs.getMaze().getPowerPillPositions().size() < totalPills / 2 && !cruiseElroy) cruiseElroy = true;
	if (ghost->isEdible() == false) { return true; }
	return false;
}
std::shared_ptr<FSMState> BlinkyUnfrightTransition::getNextState() { 
	if (cruiseElroy) return _next1; // Always returns to Chase
	switch (blinkyCS) 
	{
		case 1: return _next1; break;
		case 2: return _next2; break;
		case 3: return _next3; break;
	
		default: return _next1; break; //Safety Net
	}

}

BlinkyChaseTransition::BlinkyChaseTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool BlinkyChaseTransition::isValid(const GameState& gs) {
	if (cruiseElroy) return true;
	else {
		std::chrono::duration<double> diff = std::chrono::high_resolution_clock::now() - _start;
		double duration = gs.getMaze().getPowerPillPositions().empty() ? 7.0 : 12.0;
		if (diff.count() > duration) {
			return true;
		}
	}
	return false;
}
std::shared_ptr<FSMState> BlinkyChaseTransition::getNextState() { return _next; }

BlinkyScatterTransition::BlinkyScatterTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool BlinkyScatterTransition::isValid(const GameState& gs) {
	if (cruiseElroy) return false; 
	else {
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = end - _start;
		if (diff.count() > 20.0) {
			return true;
			// return !blinkyGoGuard;
		}
	}
	return false;
}
std::shared_ptr<FSMState> BlinkyScatterTransition::getNextState() { return _next; }

BlinkyGuardTransition::BlinkyGuardTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool BlinkyGuardTransition::isValid(const GameState& gs) {
	if (gs.getMaze().getPowerPillPositions().empty()) return false;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end - _start;
	if (diff.count() > 12.0) {
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> BlinkyGuardTransition::getNextState() { return _next; }

BlinkyUnguardTransition::BlinkyUnguardTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool BlinkyUnguardTransition::isValid(const GameState& gs) {
	if (cruiseElroy) return true; 
	auto pps = gs.getMaze().getPowerPillPositions();
	bool stillThere = std::find(pps.begin(), pps.end(), blinkyGuardTarget) != pps.end();
	if (pps.empty() || !stillThere) return true;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end - _start;
	if (diff.count() > 7.0) {
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> BlinkyUnguardTransition::getNextState() { return _next; }

#pragma endregion

#pragma region States

BlinkyChaseState::BlinkyChaseState(std::shared_ptr<Character> _character):FSMState(_character){ }
void BlinkyChaseState::onEnter(const GameState& ){ 
	// std::cout << "Blinky Chasing..." << std::endl;

	// std::dynamic_pointer_cast<Ghost>(character)->revert(); 

	blinkyCS = 1;

	for (auto& t : transitions) {
		auto st = std::dynamic_pointer_cast<BlinkyScatterTransition>(t);
		if (st) { st->_start = std::chrono::high_resolution_clock::now(); }
	}
}
Move BlinkyChaseState::onUpdate(const GameState& game) {
	
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
BlinkyChaseState::~BlinkyChaseState(){}

BlinkyGuardState::BlinkyGuardState(std::shared_ptr<Character> _character):FSMState(_character){ }
void BlinkyGuardState::onEnter(const GameState& gs){ 
	// std::cout << "Blinky Chasing..." << std::endl;
	auto pps = gs.getMaze().getPowerPillPositions();

	if (pps.empty()) { target = std::make_pair(108, 4); }
	else { 
		target = pps[rand() % pps.size()]; 
		blinkyGuardTarget = target;
	}

	blinkyCS = 3;

	for (auto& t : transitions) {
		auto stc = std::dynamic_pointer_cast<BlinkyChaseTransition>(t);
		if (stc) { stc->_start = std::chrono::high_resolution_clock::now(); }
		auto stug = std::dynamic_pointer_cast<BlinkyUnguardTransition>(t);
		if (stug) { stug->_start = std::chrono::high_resolution_clock::now(); }
	}
}
Move BlinkyGuardState::onUpdate(const GameState& game) {
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
BlinkyGuardState::~BlinkyGuardState(){}

BlinkyScatterState::BlinkyScatterState(std::shared_ptr<Character> _character) : FSMState(_character) {}
void BlinkyScatterState::onEnter(const GameState& gs) {
	// std::cout << "Blinky Scattering..." << std::endl;

	blinkyCS = 2;
	
	for (auto& t : transitions) {
		auto stg = std::dynamic_pointer_cast<BlinkyGuardTransition>(t);
		if (stg) { stg->_start = std::chrono::high_resolution_clock::now(); }
		auto stc = std::dynamic_pointer_cast<BlinkyChaseTransition>(t);
		if (stc) { stc->_start = std::chrono::high_resolution_clock::now(); }
	}
	
}
Move BlinkyScatterState::onUpdate(const GameState& game) {
	std::vector<Move> moves;
	std::pair<int, int> corner = std::make_pair(108, 4);
	const auto myPos=character->getPos();
	// std::cerr << game.getMaze().getNodePos(myPos).first << " " << game.getMaze().getNodePos(myPos).second << "\n";

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float min= moves[0] == PASS ? 10000000 : euclid2(
		game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[0])),
			corner);
	int minI=0;
	for(unsigned int i=1;i<moves.size();i++){
		auto dist=euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[i])),
			corner);
		if(dist<min){
			min=dist;
			minI=i;
		}
	}
	return moves[minI];
}
BlinkyScatterState::~BlinkyScatterState(){}

BlinkyFrightenedState::BlinkyFrightenedState(std::shared_ptr<Character> _character) : FSMState(_character) {}
void BlinkyFrightenedState::onEnter(const GameState& ) { 
	// std::cout << "Blinky Escaping..." << std::endl; 
}
Move BlinkyFrightenedState::onUpdate(const GameState& game) {
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
BlinkyFrightenedState::~BlinkyFrightenedState(){}

#pragma endregion

BlinkyStateMachine::BlinkyStateMachine(std::shared_ptr<Character> _character) : FiniteStateMachine(_character) {
	auto chaseState = std::make_shared<BlinkyChaseState>(character);
	auto guardState = std::make_shared<BlinkyGuardState>(character);
	auto scatterState = std::make_shared<BlinkyScatterState>(character);
	auto frightenedState = std::make_shared<BlinkyFrightenedState>(character);

	frightenedState->addTransition(std::make_shared<BlinkyUnfrightTransition>(chaseState, scatterState, guardState, character));

	guardState->addTransition(std::make_shared<BlinkyFrightenedTransition>(frightenedState, character));
	guardState->addTransition(std::make_shared<BlinkyChaseTransition>(chaseState, character));
	guardState->addTransition(std::make_shared<BlinkyUnguardTransition>(chaseState, character));

	chaseState->addTransition(std::make_shared<BlinkyFrightenedTransition>(frightenedState, character));
	chaseState->addTransition(std::make_shared<BlinkyScatterTransition>(scatterState, character));
	
	scatterState->addTransition(std::make_shared<BlinkyFrightenedTransition>(frightenedState, character));
	scatterState->addTransition(std::make_shared<BlinkyGuardTransition>(guardState, character));
	scatterState->addTransition(std::make_shared<BlinkyChaseTransition>(chaseState, character));
	
	states.push_back(chaseState);
	states.push_back(scatterState);
	states.push_back(frightenedState);
	states.push_back(guardState);

	initialState = chaseState;
	activeState = initialState;
}
Move BlinkyStateMachine::update(const GameState& gs) {
	if (gs.getMaze().getPillPositions().size() + gs.getMaze().getPowerPillPositions().size() < totalPills / 2 && !cruiseElroy) {
		std::cerr << "Blinky is angry...\n";
		cruiseElroy = true;
	}

	auto t = activeState->getActiveTransition(gs);
	if (t != nullptr) {
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}
BlinkyStateMachine::~BlinkyStateMachine() {}
