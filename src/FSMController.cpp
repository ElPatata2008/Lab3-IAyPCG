/*
 * FSMController.cpp
 *
 *  Created on: Apr 23, 2018
 *      Author: nbarriga
 */

#include "FSMController.h"
#include <iostream>

FSMController::FSMController(std::shared_ptr<Character> character):
	Controller(character),
	e(rand()),
	uniform_dist(0,3),
	fsm(std::make_shared<ExampleStateMachine>(character)) {
}
FSMController::~FSMController() {
	// TODO Auto-generated destructor stub
}
Move FSMController::getMove(const GameState& game){
	return fsm->update(game);
}


///////////////////////////////////PillTransition///////////////////////////////
PillTransition::PillTransition(std::shared_ptr<FSMState> next):last(0),_next(next){ }
bool PillTransition::isValid(const GameState& gs){
	int quedan=gs.getMaze().getPillPositions().size();
	if(last!=quedan && quedan%20==0){
		last =quedan;
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> PillTransition::getNextState(){
	return _next;
}

NonFrightenedTransition::NonFrightenedTransition(std::shared_ptr<FSMState> next1, std::shared_ptr<FSMState> next2, std::shared_ptr<Character> character):_next1(next1), _next2(next2), _character(character){}
bool NonFrightenedTransition::isValid(const GameState& gs) {
	Ghost *ghost = dynamic_cast<Ghost*>(_character.get());
	if (ghost->isEdible() == false){
		switch (rand() % 2)
		{
			case 0: _next = _next1; break;
			case 1: _next = _next2; break;
			default: _next = _next1; break;
		}
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> NonFrightenedTransition::getNextState() { return _next; }

FrightenedTransition::FrightenedTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character){}
bool FrightenedTransition::isValid(const GameState& gs) {
	Ghost *ghost = dynamic_cast<Ghost*>(_character.get());
	if (ghost->isEdible() == true){
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> FrightenedTransition::getNextState() { return _next; }

ChaseTransition::ChaseTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool ChaseTransition::isValid(const GameState& gs) {
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end - _start;
	// std::cout << diff.count() << std::endl;
	if (diff.count() > 7.0) {
		// _start = std::chrono::high_resolution_clock::now();
		return true;
	}

	return false;
}
std::shared_ptr<FSMState> ChaseTransition::getNextState() { return _next; }

ScatterTransition::ScatterTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character):_next(next), _character(character) {}
bool ScatterTransition::isValid(const GameState& gs) {

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end - _start;
	// std::cout << diff.count() << std::endl;
	if (diff.count() > 20.0) {
		// _start = std::chrono::high_resolution_clock::now();
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> ScatterTransition::getNextState() { 
	// _start = std::chrono::high_resolution_clock::now();
	return _next; 
}

///////////////////////////////ChaseState///////////////////////////////////////
ChaseState::ChaseState(std::shared_ptr<Character> _character):FSMState(_character){ }
void ChaseState::onEnter(const GameState& ){ 
	std::cout << "Chasing..." << std::endl;

	std::dynamic_pointer_cast<Ghost>(character)->revert(); 

	for (auto& t : transitions) {
		auto st = std::dynamic_pointer_cast<ScatterTransition>(t);
		if (st) {
			st->_start = std::chrono::high_resolution_clock::now();
		}
	}
}
Move ChaseState::onUpdate(const GameState& game) {
	
	std::vector<Move> moves;
	const auto pacmanCoord=game.getMaze().getNodePos(game.getPacmanPos());
	const auto myPos=character->getPos();
	//const auto myCoord=game.getMaze().getNodePos(myPos);

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float min=euclid2(
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
ChaseState::~ChaseState(){}

ScatterState::ScatterState(std::shared_ptr<Character> _character) : FSMState(_character) {}
void ScatterState::onEnter(const GameState& ) {
	std::cout << "Scattering..." << std::endl;
	
	for (auto& t : transitions) {
		auto st = std::dynamic_pointer_cast<ChaseTransition>(t);
		if (st) {
			st->_start = std::chrono::high_resolution_clock::now();
		}
	}
	
}
Move ScatterState:: onUpdate(const GameState& game) {

	std::vector<Move> moves;
	std::pair<int, int> corner = {-20, -20};
	// const auto cornerCoord={-20, -20};
	const auto myPos=character->getPos();
	//const auto myCoord=game.getMaze().getNodePos(myPos);

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float min=euclid2(
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
ScatterState::~ScatterState(){}

FrightenedState::FrightenedState(std::shared_ptr<Character> _character) : FSMState(_character) {}
void FrightenedState::onEnter(const GameState& ) { 
	std::cout << "Escaping..." << std::endl;
}
Move FrightenedState::onUpdate(const GameState& game) {
	// std::cout << "FrightenedState" << std::endl;

	std::vector<Move> moves;
	const auto pacmanCoord=game.getMaze().getNodePos(game.getPacmanPos());
	const auto myPos=character->getPos();
	//const auto myCoord=game.getMaze().getNodePos(myPos);

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float max=euclid2(
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
FrightenedState::~FrightenedState(){}

/////////////////////////////////////BlinkyStateMachine/////////////////////////////
ExampleStateMachine::ExampleStateMachine(std::shared_ptr<Character> _character):FiniteStateMachine(_character){
	auto chaseState = std::make_shared<ChaseState>(character);
	auto scatterState = std::make_shared<ScatterState>(character);
	auto frightenedState = std::make_shared<FrightenedState>(character);

	frightenedState->addTransition(std::make_shared<NonFrightenedTransition>(chaseState, scatterState, character));

	chaseState->addTransition(std::make_shared<FrightenedTransition>(frightenedState, character));
	chaseState->addTransition(std::make_shared<ScatterTransition>(scatterState, character));
	
	scatterState->addTransition(std::make_shared<FrightenedTransition>(frightenedState, character));
	scatterState->addTransition(std::make_shared<ChaseTransition>(chaseState, character));
	
	states.push_back(chaseState);
	states.push_back(frightenedState);

	initialState = chaseState;
	activeState = initialState;
}

Move ExampleStateMachine::update(const GameState& gs){
	auto t = activeState->getActiveTransition(gs);
	if(t != nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}

ExampleStateMachine::~ExampleStateMachine(){ }


