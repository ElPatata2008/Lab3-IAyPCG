#ifndef SUECONTROLLER_H_
#define SUECONTROLLER_H_

#include "Controller.h"
#include <random>
#include <chrono>
#include "FSM.h"

class SueStateMachine;

class SueController: public Controller {
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
	std::shared_ptr<SueStateMachine> fsm;
public:
	SueController(std::shared_ptr<Character> character);
	virtual ~SueController();
	virtual Move getMove(const GameState& game)override;
};

#pragma region Transitions

class SueFrightenedTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public: 
	SueFrightenedTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class SueUnfrightTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next1;
	std::shared_ptr<FSMState> _next2;
	std::shared_ptr<FSMState> _next3;
public: 
	SueUnfrightTransition(std::shared_ptr<FSMState> next1, std::shared_ptr<FSMState> next2, std::shared_ptr<FSMState> next3, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class SueGuardTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public:
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration> _start;
	SueGuardTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class SueUnguardTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next1;
	std::shared_ptr<FSMState> _next2;
	int state = 1;
public:
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration> _start;
	SueUnguardTransition(std::shared_ptr<FSMState> next1, std::shared_ptr<FSMState> next2, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class SueChaseTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
	public:
	SueChaseTransition(std::shared_ptr<FSMState> next,  std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class SueScatterTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
	public:
	SueScatterTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

#pragma endregion

#pragma region States

class SueFrightenedState : public FSMState {

public: 
	SueFrightenedState(std::shared_ptr<Character> character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~SueFrightenedState();
};

class SueScatterState : public FSMState {

public: 
	SueScatterState(std::shared_ptr<Character> character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~SueScatterState();
};

class SueGuardState : public FSMState {
private:
	std::pair<int, int> target;
public: 
	SueGuardState(std::shared_ptr<Character> character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~SueGuardState();
};

class SueChaseState : public FSMState {

public:
	SueChaseState(std::shared_ptr<Character> character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~SueChaseState();
};

#pragma endregion

class SueStateMachine : public FiniteStateMachine {

public:
	SueStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~SueStateMachine();
};

#endif