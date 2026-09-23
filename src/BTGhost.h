#ifndef BTGHOSTCONTROLLER_H_
#define BTGHOSTCONTROLLER_H_

#include "Controller.h"
#include "BehaviorTree.h"
#include <chrono>

#include <random>
class BTGhostInfo {
	static BTGhostInfo *btghostInfo;
	BTGhostInfo(){}

public:
	static BTGhostInfo* getInfo() {
		if (btghostInfo == nullptr) btghostInfo = new BTGhostInfo();
		return btghostInfo;
	}
	const GameState* in_gamestate;
	Move out_move;
	std::shared_ptr<Character> in_character;
};

class BTGhost: public Controller {
private: 
	std::shared_ptr<Composite> root;

public:
	BTGhost(std::shared_ptr<Character> character);
	virtual ~BTGhost();
	virtual Move getMove(const GameState& game)override;
};

#pragma region Behaviors

class BTGhostChase : public Behavior {
public:
	virtual Status update() override;
};

class BTGhostFrightened : public Behavior {
private:
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
public:
	virtual Status update() override;
	BTGhostFrightened();
};

class BTGhostCenter : public Behavior {
private: 
	std::pair<int, int> target;
public: 
	virtual Status update() override;
	BTGhostCenter();
};

#pragma endregion

#pragma region Conditions

class BTGhostPowerpill : public Behavior {
public:
	virtual Status update() override;
};

class BTGhostTimeOut : public Behavior {
private: 
	std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
public:
	virtual Status update() override;
	BTGhostTimeOut ();
	float elapsedTime() const; 
};

class BTGhostCenterMode : public Behavior {
private:
	std::shared_ptr<BTGhostTimeOut> _timer;
public:
	BTGhostCenterMode(std::shared_ptr<BTGhostTimeOut> timer);
	virtual Status update() override;
};

#pragma endregion

#endif