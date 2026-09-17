#include "SueController.h"
#include <iostream>

int SueCS = 1;
int SueCowardnessRadius = 200;

std::chrono::time_point<std::chrono::high_resolution_clock> sueGuardCycleStart = std::chrono::high_resolution_clock::now();
std::pair<int, int> sueGuardTarget = {-1, -1};

SueController::SueController(std::shared_ptr<Character> character):
	Controller(character),
	e(rand()),
	uniform_dist(0,3),
	fsm(std::make_shared<SueStateMachine>(character)){
}
SueController::~SueController() {}
Move SueController::getMove(const GameState& game){ return fsm->update(game); }

#pragma region Transitions

SueFrightenedTransition::SueFrightenedTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character){ }
bool SueFrightenedTransition::isValid(const GameState& gs) {
	Ghost *ghost = dynamic_cast<Ghost*>(_character.get());
	if (ghost->isEdible() == true){ return true; }
	return false;
}
std::shared_ptr<FSMState> SueFrightenedTransition::getNextState() { return _next; }

SueUnfrightTransition::SueUnfrightTransition(std::shared_ptr<FSMState> next1, std::shared_ptr<FSMState> next2, std::shared_ptr<FSMState> next3, std::shared_ptr<Character> character): _next1(next1), _next2(next2), _next3(next3), _character(character) {}
bool SueUnfrightTransition::isValid(const GameState& gs) {
	Ghost *ghost = dynamic_cast<Ghost*>(_character.get());
	if (ghost->isEdible() == false) { return true; }
	return false;
}
std::shared_ptr<FSMState> SueUnfrightTransition::getNextState() { 
	switch (SueCS)
	{
		case 1: return _next1; break;
		case 2: return _next2; break;
		case 3: return _next3; break;
		
		default: return _next1; break; 
	}
}

SueChaseTransition::SueChaseTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool SueChaseTransition::isValid(const GameState& gs) {
	
	const auto myPos = gs.getMaze().getNodePos(_character->getPos());
	auto pacmanPos = gs.getMaze().getNodePos(gs.getPacmanPos());

	float dist = euclid2(myPos, pacmanPos);
	
	if (dist > SueCowardnessRadius) {
		return true;
	}

	return false;
}
std::shared_ptr<FSMState> SueChaseTransition::getNextState() { return _next; }

SueScatterTransition::SueScatterTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool SueScatterTransition::isValid(const GameState& gs) {
	const auto myPos = gs.getMaze().getNodePos(_character->getPos());
	auto pacmanPos = gs.getMaze().getNodePos(gs.getPacmanPos());

	float dist = euclid2(myPos, pacmanPos);
	
	if (dist < SueCowardnessRadius) {
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> SueScatterTransition::getNextState() { return _next; }

SueGuardTransition::SueGuardTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool SueGuardTransition::isValid(const GameState& gs) {
	Ghost *ghost = dynamic_cast<Ghost*>(_character.get());
	if (ghost->isEdible()) return false;
	if (gs.getMaze().getPowerPillPositions().empty()) return false;

	auto diff = std::chrono::high_resolution_clock::now() - sueGuardCycleStart;
	return diff.count() > 20;
}
std::shared_ptr<FSMState> SueGuardTransition::getNextState() { return _next; }

SueUnguardTransition::SueUnguardTransition(std::shared_ptr<FSMState> next1, std::shared_ptr<FSMState> next2, std::shared_ptr<Character> character):_next1(next1), _next2(next2), _character(character) {}
bool SueUnguardTransition::isValid(const GameState& gs) {
	auto pps = gs.getMaze().getPowerPillPositions();
	bool stillthere = std::find(pps.begin(), pps.end(), sueGuardTarget) != pps.end();
	auto diff = std::chrono::high_resolution_clock::now() - _start;

	auto myCoord = gs.getMaze().getNodePos(_character->getPos());
	auto pacmanCoord = gs.getMaze().getNodePos(gs.getPacmanPos());
	state = euclid2(myCoord, pacmanCoord) > SueCowardnessRadius ? 1 : 2;

	return pps.empty() || !stillthere || diff.count() > 15.0;
}
std::shared_ptr<FSMState> SueUnguardTransition::getNextState() { return state == 1 ? _next1 : _next2; }

#pragma endregion

#pragma region States

SueChaseState::SueChaseState(std::shared_ptr<Character> _character):FSMState(_character){ }
void SueChaseState::onEnter(const GameState& ){ 
	// std::cout << "Sue Chasing..." << std::endl;

	// std::dynamic_pointer_cast<Ghost>(character)->revert(); 

	SueCS = 1;
}
Move SueChaseState::onUpdate(const GameState& game) {
	
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
SueChaseState::~SueChaseState(){}

SueScatterState::SueScatterState(std::shared_ptr<Character> _character) : FSMState(_character) {}
void SueScatterState::onEnter(const GameState& ) {
	// std::cout << "Sue Scattering..." << std::endl;

	SueCS = 2;	
}
Move SueScatterState::onUpdate(const GameState& game) {
	std::vector<Move> moves;
	std::pair<int, int> corner = std::make_pair(0, 116);
	const auto myPos=character->getPos();

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
SueScatterState::~SueScatterState(){}

SueGuardState::SueGuardState(std::shared_ptr<Character> _character) : FSMState(_character) {}
void SueGuardState::onEnter(const GameState& gs) {
	// std::cout << "Sue Scattering..." << std::endl;

	SueCS = 3;	

	auto myCoord = gs.getMaze().getNodePos(character->getPos());
	auto pps = gs.getMaze().getPowerPillPositions();

	target = *std::min_element(pps.begin(), pps.end(), [&](auto &a, auto &b) {
		return euclid2(myCoord, a) < euclid2(myCoord, b);
	});

	sueGuardTarget = target;
	sueGuardCycleStart = std::chrono::high_resolution_clock::now();

}
Move SueGuardState::onUpdate(const GameState& game) {
	std::vector<Move> moves;
	const auto myPos=character->getPos();

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
SueGuardState::~SueGuardState(){}

SueFrightenedState::SueFrightenedState(std::shared_ptr<Character> _character) : FSMState(_character) {}
void SueFrightenedState::onEnter(const GameState& ) {
	// std::cout << "Sue Escaping..." << std::endl; 
}
Move SueFrightenedState::onUpdate(const GameState& game) {
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
			pacmanCoord
		);
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
SueFrightenedState::~SueFrightenedState(){}

#pragma endregion

SueStateMachine::SueStateMachine(std::shared_ptr<Character> _character) : FiniteStateMachine(_character) {
	auto chaseState = std::make_shared<SueChaseState>(character);
	auto guardState = std::make_shared<SueGuardState>(character);
	auto scatterState = std::make_shared<SueScatterState>(character);
	auto frightenedState = std::make_shared<SueFrightenedState>(character);

	frightenedState->addTransition(std::make_shared<SueUnfrightTransition>(chaseState, scatterState, guardState, character));

	guardState->addTransition(std::make_shared<SueFrightenedTransition>(frightenedState, character));
	guardState->addTransition(std::make_shared<SueUnguardTransition>(chaseState, scatterState, character));

	chaseState->addTransition(std::make_shared<SueFrightenedTransition>(frightenedState, character));
	chaseState->addTransition(std::make_shared<SueScatterTransition>(scatterState, character));
	
	scatterState->addTransition(std::make_shared<SueFrightenedTransition>(frightenedState, character));
	scatterState->addTransition(std::make_shared<SueChaseTransition>(chaseState, character));
	
	states.push_back(chaseState);
	states.push_back(scatterState);
	states.push_back(frightenedState);
	states.push_back(guardState);

	initialState = chaseState;
	activeState = initialState;
}
Move SueStateMachine::update(const GameState& gs) {
	auto t = activeState->getActiveTransition(gs);
	if (t != nullptr) {
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}
SueStateMachine::~SueStateMachine() {}
