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

Move 
FSMController::getMove(const GameState& game){
	return fsm->update(game);
}


///////////////////////////////////PillTransition///////////////////////////////
PillTransition::PillTransition(std::shared_ptr<FSMState> next):last(0),_next(next){
}

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



///////////////////////////////ChaseState///////////////////////////////////////
ChaseState::ChaseState(std::shared_ptr<Character> _character):FSMState(_character){ }

void ChaseState::onEnter(const GameState& ){ std::dynamic_pointer_cast<Ghost>(character)->revert(); }

Move ChaseState::onUpdate(const GameState& game){
	// Ghost *ghost = dynamic_cast<Ghost*>(character.get());
	// std::cout << "Frightened" << ghost->isEdible() << std::endl;

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

FrightenedState::FrightenedState(std::shared_ptr<Character> _character) : FSMState(_character) {}

void FrightenedState::onEnter(const GameState& game) { std::dynamic_pointer_cast<Ghost>(character)->revert(); }

Move FrightenedState::onUpdate(const GameState& game) {
	std::cout << "FrightenedState" << std::endl;
}
FrightenedState::~FrightenedState(){}

/////////////////////////////////////BlinkyStateMachine/////////////////////////////
ExampleStateMachine::ExampleStateMachine(std::shared_ptr<Character> _character):FiniteStateMachine(_character){
	initialState = std::make_shared<ChaseState>(character);
	// std::shared_ptr<FSMState> frightenedState = std::make_shared<FrightenedState>(character);
	activeState=initialState;
	// states.push_back(frightenedState);
	states.push_back(initialState);
	// frightenedState->addTransition(std::make_shared<PillTransition>(activeState));
	// activeState->addTransition(std::make_shared<PillTransition>(frightenedState)); // Arreglar
	activeState->addTransition(std::make_shared<PillTransition>(activeState)); // Arreglar
}



Move ExampleStateMachine::update(const GameState& gs){
	auto t = activeState->getActiveTransition(gs);
	if(t != nullptr){
		std::cout << t->getNextState() << std::endl;
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}


ExampleStateMachine::~ExampleStateMachine(){

}


