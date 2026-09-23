#include "InkyController.h"
#include <iostream>

InkyInfo* InkyInfo::inkyInfo = nullptr;

InkyController::InkyController(std::shared_ptr<Character> character): Controller(character), root(std::make_shared<Selector>()){
	auto filterFrightened = std::make_shared<Filter>();
	auto filterScatter = std::make_shared<Filter>();
	auto filterChase = std::make_shared<Filter>();

	auto timeout = std::make_shared<InkyTimeOut>();

	filterFrightened->addCondition(std::make_shared<InkyPowerpill>());
	filterFrightened->addAction(std::make_shared<InkyFrightened>());

	filterScatter->addCondition(timeout);
	filterScatter->addAction(std::make_shared<InkyScatter>());

	filterChase->addCondition(std::make_shared<Invertor>(timeout));
	filterChase->addAction(std::make_shared<InkyChase>());

	root->addChild(filterFrightened);
	root->addChild(filterScatter);
	root->addChild(filterChase);
}

InkyController::~InkyController() { }

Move InkyController::getMove(const GameState& game){
	InkyInfo::getInfo()->in_character = character;
	InkyInfo::getInfo()->in_gamestate = &game;
	root->tick();

	return InkyInfo::getInfo()->out_move;
}

#pragma region Conditions

InkyTimeOut::InkyTimeOut() : Behavior() { lastTime = std::chrono::high_resolution_clock::now(); }
Status InkyTimeOut::update() {
	return (int)elapsedTime() % 27 < 7 ? BH_SUCCESS : BH_FAILURE;
}
float InkyTimeOut::elapsedTime() const {
	std::chrono::duration<float> time = std::chrono::high_resolution_clock::now() - lastTime;
	return time.count();
}

Status InkyPowerpill::update() {
	auto character = InkyInfo::getInfo()->in_character;
	auto ghost = dynamic_cast<Ghost*>(character.get());

	return ghost != nullptr && ghost->isEdible() ? BH_SUCCESS : BH_FAILURE;
}

#pragma endregion

#pragma region Actions

Status InkyChase::update() {
	auto character = InkyInfo::getInfo()->in_character;
	auto gs = InkyInfo::getInfo()->in_gamestate;
	
	auto targetDir = gs->getPacmanDir();
	int aheadPos = gs->getPacmanPos();

	if (targetDir != PASS) {
		Move m = static_cast<Move>(targetDir);
		for (int i = 0; i < 2; i++) {
			int nextPos = gs->getMaze().getNeighbour(aheadPos, m);
			if (nextPos == -1) break;
	
			aheadPos = nextPos;
		}
	}

	auto aheadCoord = gs->getMaze().getNodePos(aheadPos);

	auto blinkyPos = gs->getGhostsPos(0);
	auto blinkyCoord = gs->getMaze().getNodePos(blinkyPos);

	auto dx = aheadCoord.first - blinkyCoord.first;
	auto dy = aheadCoord.second - blinkyCoord.second;

	auto target = std::make_pair(blinkyCoord.first + 2 * dx, blinkyCoord.second + 2 * dy);


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
	InkyInfo::getInfo()->out_move = minMove;
	return BH_SUCCESS;
}

InkyScatter::InkyScatter() : Behavior() { target = std::make_pair(108, 116); }
Status InkyScatter::update() {
	if(target.first == -1){
		target = InkyInfo::getInfo()->in_gamestate->getMaze().getPowerPillPositions()[0];
	}

	auto character = InkyInfo::getInfo()->in_character;
	auto gs = InkyInfo::getInfo()->in_gamestate;

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
	InkyInfo::getInfo()->out_move = minMove;
	return BH_SUCCESS;
}

InkyFrightened::InkyFrightened() : Behavior(), e(rand()), uniform_dist(0,3) {}
Status InkyFrightened::update() {
	// std::cerr << " Frightened \n" ;
	auto character = InkyInfo::getInfo()->in_character;
	auto gs = InkyInfo::getInfo()->in_gamestate;
	std::vector<Move> moves;
	if(character->getDirection()==PASS) {
		moves=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}
	Move m = moves[rand()%moves.size()];
	InkyInfo::getInfo()->out_move = m;
	return BH_SUCCESS;
}

#pragma endregion
