#ifndef BLINKYCONTROLLER_H_
#define BLINKYCONTROLLER_H_

#include "Controller.h"
#include <random>
#include <chrono>
#include "FSM.h"

class BlinkyStateMachine;

class BlinkyController: public Controller {
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
	std::shared_ptr<BlinkyStateMachine> fsm;
public:
	BlinkyController(std::shared_ptr<Character> character);
	virtual ~BlinkyController();
	virtual Move getMove(const GameState& game)override;
};

#pragma region Transitions

class BlinkyFrightenedTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public: 
	BlinkyFrightenedTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class BlinkyUnfrightTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next1;
	std::shared_ptr<FSMState> _next2;
public: 
	BlinkyUnfrightTransition(
		std::shared_ptr<FSMState> next1, 
		std::shared_ptr<FSMState> next2, 
		std::shared_ptr<Character> character
	);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class BlinkyChaseTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
	public:
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration> _start;
	BlinkyChaseTransition(std::shared_ptr<FSMState> next,  std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class BlinkyScatterTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
	public:
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration> _start;
	BlinkyScatterTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

#pragma endregion

#pragma region States

class BlinkyFrightenedState : public FSMState {

public: 
	BlinkyFrightenedState(std::shared_ptr<Character> character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~BlinkyFrightenedState();
};

class BlinkyScatterState : public FSMState {

public: 
	BlinkyScatterState(std::shared_ptr<Character> character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~BlinkyScatterState();
};

class BlinkyChaseState : public FSMState {

public:
	BlinkyChaseState(std::shared_ptr<Character> character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~BlinkyChaseState();
};

#pragma endregion

class BlinkyStateMachine : public FiniteStateMachine {

public:
	BlinkyStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~BlinkyStateMachine();
};

#endif