#ifndef PINKYCONTROLLER_H_
#define PINKYCONTROLLER_H_

#include "Controller.h"
#include "BehaviorTree.h"
#include <chrono>

#include <random>
class PinkyInfo {
	static PinkyInfo *pinkyInfo;
	PinkyInfo(){}

public:
	static PinkyInfo* getInfo() {
		if (pinkyInfo == nullptr) pinkyInfo = new PinkyInfo();
		return pinkyInfo;
	}
	const GameState* in_gamestate;
	Move out_move;
	std::shared_ptr<Character> in_character;
};

class PinkyController: public Controller {
private: 
	std::shared_ptr<Composite> root;

public:
	PinkyController(std::shared_ptr<Character> character);
	virtual ~PinkyController();
	virtual Move getMove(const GameState& game)override;
};

#pragma region Behaviors

class PinkyChase : public Behavior {
public:
	virtual Status update() override;
};

class PinkyFrightened : public Behavior {
private:
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
public:
	virtual Status update() override;
	PinkyFrightened();
};

class PinkyScatter : public Behavior {
private:
	std::pair<int, int> target;
public:
	virtual Status update() override;
	PinkyScatter();
};

class PinkyCenter : public Behavior {
private: 
	std::pair<int, int> target;
public: 
	virtual Status update() override;
	PinkyCenter();
};

#pragma endregion

#pragma region Conditions

class PinkyPowerpill : public Behavior {
public:
	virtual Status update() override;
};

class PinkyTimeOut : public Behavior {
private: 
	std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
public:
	virtual Status update() override;
	PinkyTimeOut ();
	float elapsedTime() const; 
};

class PinkyScatterMode : public Behavior {
private:
	std::shared_ptr<PinkyTimeOut> _timer;
public:
	PinkyScatterMode(std::shared_ptr<PinkyTimeOut> timer);
	virtual Status update() override;
};

class PinkyCenterMode : public Behavior {
private:
	std::shared_ptr<PinkyTimeOut> _timer;
public:
	PinkyCenterMode(std::shared_ptr<PinkyTimeOut> timer);
	virtual Status update() override;
};

#pragma endregion

#endif