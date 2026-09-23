#include "BTGhost.h"
#include <iostream>

BTGhostInfo* BTGhostInfo::btghostInfo = nullptr;

BTGhost::BTGhost(std::shared_ptr<Character> character): Controller(character), root(std::make_shared<Selector>()){
	auto filterFrightened = std::make_shared<Filter>();
	auto filterCenter = std::make_shared<Filter>();
	auto filterChase = std::make_shared<Filter>();

	auto timeout = std::make_shared<BTGhostTimeOut>();

	filterFrightened->addCondition(std::make_shared<BTGhostPowerpill>());
	filterFrightened->addAction(std::make_shared<BTGhostFrightened>());

	filterCenter->addCondition(timeout);
	filterCenter->addAction(std::make_shared<BTGhostCenter>());

	filterChase->addCondition(std::make_shared<Invertor>(timeout));
	filterChase->addAction(std::make_shared<BTGhostChase>());

	root->addChild(filterFrightened);
	root->addChild(filterCenter);
	root->addChild(filterChase);
}

BTGhost::~BTGhost() { }

Move BTGhost::getMove(const GameState& game){
	BTGhostInfo::getInfo()->in_character = character;
	BTGhostInfo::getInfo()->in_gamestate = &game;
	root->tick();

	return BTGhostInfo::getInfo()->out_move;
}

#pragma region Conditions

BTGhostTimeOut::BTGhostTimeOut() : Behavior() { lastTime = std::chrono::high_resolution_clock::now(); }
Status BTGhostTimeOut::update() { return (int)elapsedTime() % 27 < 7 ? BH_SUCCESS : BH_FAILURE; }
float BTGhostTimeOut::elapsedTime() const {
	std::chrono::duration<float> time = std::chrono::high_resolution_clock::now() - lastTime;
	return time.count();
}

Status BTGhostPowerpill::update() {
	auto character = BTGhostInfo::getInfo()->in_character;
	auto ghost = dynamic_cast<Ghost*>(character.get());

	return ghost != nullptr && ghost->isEdible() ? BH_SUCCESS : BH_FAILURE;
}

#pragma endregion

#pragma region Actions

Status BTGhostChase::update() {
	auto character = BTGhostInfo::getInfo()->in_character;
	auto gs = BTGhostInfo::getInfo()->in_gamestate;
	auto target = gs->getMaze().getNodePos(gs->getPacmanPos());

	float min=1000000000;
	Move minMove=PASS;
	std::vector<Move> moves;
	if(character->getDirection()==PASS) {
		moves=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	for(auto move:moves) {
		if(move==PASS) {
			break;
		}
		float dist = euclid2(target,gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(),move)));
		if(dist<min) {
			min=dist;
			minMove=move;
		}
	}
	BTGhostInfo::getInfo()->out_move = minMove;
	return BH_SUCCESS;
}

BTGhostCenter::BTGhostCenter() : Behavior() { target = std::make_pair(54, 60);  }
Status BTGhostCenter::update() {
	if(target.first == -1){
		target = BTGhostInfo::getInfo()->in_gamestate->getMaze().getPowerPillPositions()[0];
	}

	auto character = BTGhostInfo::getInfo()->in_character;
	auto gs = BTGhostInfo::getInfo()->in_gamestate;

	Move minMove=PASS;
	std::vector<Move> moves;
	if(character->getDirection()==PASS) {
		moves=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	float min=100000000;
	for(auto move:moves) {
		if(move==PASS) {
			break;
		}
		float dist = euclid2(target,gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(),move)));
		if(dist<min) {
			min=dist;
			minMove=move;
		}
	}
	BTGhostInfo::getInfo()->out_move = minMove;
	return BH_SUCCESS;
}

BTGhostFrightened::BTGhostFrightened() : Behavior(), e(rand()), uniform_dist(0,3) {}
Status BTGhostFrightened::update() {
	// std::cerr << " Frightened \n" ;
	auto character = BTGhostInfo::getInfo()->in_character;
	auto gs = BTGhostInfo::getInfo()->in_gamestate;
	std::vector<Move> moves;
	if(character->getDirection()==PASS) {
		moves=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}
	Move m = moves[rand()%moves.size()];
	BTGhostInfo::getInfo()->out_move = m;
	return BH_SUCCESS;
}


#pragma endregion
