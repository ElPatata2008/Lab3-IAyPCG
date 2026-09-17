#include "PinkyController.h"
#include <iostream>

int pinkyLastCycle = -1;
int pinkyUseCenter = false;

void pinkyUpdateMode(float s) {
	int cycle = (int)s / 27;
	if (cycle != pinkyLastCycle) {
		pinkyLastCycle = cycle;
		pinkyUseCenter = (rand() % 2 == 0);
	}
}

PinkyInfo* PinkyInfo::pinkyInfo = nullptr;

PinkyController::PinkyController(std::shared_ptr<Character> character): Controller(character), root(std::make_shared<Selector>()){
	auto filterFrightened = std::make_shared<Filter>();
	auto filterScatter = std::make_shared<Filter>();
	auto filterCenter = std::make_shared<Filter>();
	auto filterChase = std::make_shared<Filter>();

	auto timeout = std::make_shared<PinkyTimeOut>();

	filterFrightened->addCondition(std::make_shared<PinkyPowerpill>());
	filterFrightened->addAction(std::make_shared<PinkyFrightened>());

	filterCenter->addCondition(std::make_shared<PinkyCenterMode>(timeout));
	filterCenter->addAction(std::make_shared<PinkyCenter>());

	filterScatter->addCondition(std::make_shared<PinkyScatterMode>(timeout));
	filterScatter->addAction(std::make_shared<PinkyScatter>());

	filterChase->addCondition(std::make_shared<Invertor>(timeout));
	filterChase->addAction(std::make_shared<PinkyChase>());

	root->addChild(filterFrightened);
	root->addChild(filterCenter);
	root->addChild(filterScatter);
	root->addChild(filterChase);
}

PinkyController::~PinkyController() { }

Move PinkyController::getMove(const GameState& game){
	PinkyInfo::getInfo()->in_character = character;
	PinkyInfo::getInfo()->in_gamestate = &game;
	root->tick();

	return PinkyInfo::getInfo()->out_move;
}

#pragma region Conditions

PinkyTimeOut::PinkyTimeOut() : Behavior() { lastTime = std::chrono::high_resolution_clock::now(); }
Status PinkyTimeOut::update() {
	return (int)elapsedTime() % 27 < 7 ? BH_SUCCESS : BH_FAILURE;
}
float PinkyTimeOut::elapsedTime() const {
	std::chrono::duration<float> time = std::chrono::high_resolution_clock::now() - lastTime;
	return time.count();
}

PinkyScatterMode::PinkyScatterMode(std::shared_ptr<PinkyTimeOut> timer) : Behavior(), _timer(timer) {}
Status PinkyScatterMode::update() {
	float t = _timer->elapsedTime();
	pinkyUpdateMode(t);
	bool inTime = ((int)t % 27) < 7;
	return (inTime && !pinkyUseCenter) ? BH_SUCCESS : BH_FAILURE;
}

PinkyCenterMode::PinkyCenterMode(std::shared_ptr<PinkyTimeOut> timer) : Behavior(), _timer(timer) {}
Status PinkyCenterMode::update() {
	float t = _timer->elapsedTime();
	pinkyUpdateMode(t);
	bool inTime = ((int)t % 27) < 7;
	return (inTime && pinkyUseCenter) ? BH_SUCCESS : BH_FAILURE;
}

Status PinkyPowerpill::update() {
	auto character = PinkyInfo::getInfo()->in_character;
	auto ghost = dynamic_cast<Ghost*>(character.get());

	return ghost != nullptr && ghost->isEdible() ? BH_SUCCESS : BH_FAILURE;
}

#pragma endregion

#pragma region Actions

Status PinkyChase::update() {
	auto character = PinkyInfo::getInfo()->in_character;
	auto gs = PinkyInfo::getInfo()->in_gamestate;
	
	auto targetDir = gs->getPacmanDir();
	int targetPos = gs->getPacmanPos();

	if (targetDir != PASS) {
		Move m = static_cast<Move>(targetDir);
		for (int i = 0; i < 4; i++) {
			int nextPos = gs->getMaze().getNeighbour(targetPos, m);
			if (nextPos == -1) break;
			targetPos = nextPos;
		}
	}

	auto target = gs->getMaze().getNodePos(targetPos);

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
	PinkyInfo::getInfo()->out_move = minMove;
	return BH_SUCCESS;
}

PinkyScatter::PinkyScatter() : Behavior() { target = std::make_pair(0, 4); }
Status PinkyScatter::update() {
	if(target.first == -1){
		target = PinkyInfo::getInfo()->in_gamestate->getMaze().getPowerPillPositions()[0];
	}

	auto character = PinkyInfo::getInfo()->in_character;
	auto gs = PinkyInfo::getInfo()->in_gamestate;

	// std::cerr << gs->getMaze().getNodePos(character->getPos()).first << ", " << gs->getMaze().getNodePos(character->getPos()).second << "\n";

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
	PinkyInfo::getInfo()->out_move = minMove;
	return BH_SUCCESS;
}

PinkyCenter::PinkyCenter() : Behavior() { target = std::make_pair(54, 60);  }
Status PinkyCenter::update() {
	if(target.first == -1){
		target = PinkyInfo::getInfo()->in_gamestate->getMaze().getPowerPillPositions()[0];
	}

	auto character = PinkyInfo::getInfo()->in_character;
	auto gs = PinkyInfo::getInfo()->in_gamestate;

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
	PinkyInfo::getInfo()->out_move = minMove;
	return BH_SUCCESS;
}

PinkyFrightened::PinkyFrightened() : Behavior(), e(rand()), uniform_dist(0,3) {}
Status PinkyFrightened::update() {
	// std::cerr << " Frightened \n" ;
	auto character = PinkyInfo::getInfo()->in_character;
	auto gs = PinkyInfo::getInfo()->in_gamestate;
	std::vector<Move> moves;
	if(character->getDirection()==PASS) {
		moves=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}
	Move m = moves[rand()%moves.size()];
	PinkyInfo::getInfo()->out_move = m;
	return BH_SUCCESS;
}


#pragma endregion
