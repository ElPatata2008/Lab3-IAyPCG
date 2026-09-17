#ifndef INKYCONTROLLER_H_
#define INKYCONTROLLER_H_

#include "Controller.h"
#include "BehaviorTree.h"
#include <chrono>


#include <random>
class InkyInfo {
	static InkyInfo *inkyInfo;
	InkyInfo(){}

public:
	static InkyInfo* getInfo() {
		if (inkyInfo == nullptr) inkyInfo = new InkyInfo();
		return inkyInfo;
	}
	const GameState* in_gamestate;
	Move out_move;
	std::shared_ptr<Character> in_character;
};

class InkyController: public Controller {
private: 
	std::shared_ptr<Composite> root;

public:
	InkyController(std::shared_ptr<Character> character);
	virtual ~InkyController();
	virtual Move getMove(const GameState& game)override;
};

#pragma region Behaviors

class InkyChase : public Behavior {
public:
	virtual Status update() override;
};

class InkyFrightened : public Behavior {
private:
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
public:
	virtual Status update() override;
	InkyFrightened();
};

class InkyScatter : public Behavior {
private:
	std::pair<int, int> target;
public:
	virtual Status update() override;
	InkyScatter();
};

class InkyCenter : public Behavior {
private: 
	std::pair<int, int> target;
public: 
	virtual Status update() override;
	InkyCenter();
};

#pragma endregion

#pragma region Conditions

class InkyPowerpill : public Behavior {
public:
	virtual Status update() override;
};

class InkyTimeOut : public Behavior {
private: 
	std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
public:
	virtual Status update() override;
	InkyTimeOut ();
	float elapsedTime() const;
};

class InkyScatterMode : public Behavior {
private:
	std::shared_ptr<InkyTimeOut> _timer;
public:
	InkyScatterMode(std::shared_ptr<InkyTimeOut> timer);
	virtual Status update() override;
};

class InkyCenterMode : public Behavior {
private:
	std::shared_ptr<InkyTimeOut> _timer;
public:
	InkyCenterMode(std::shared_ptr<InkyTimeOut> timer);
	virtual Status update() override;
};

#pragma endregion


#endif