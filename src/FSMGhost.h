#ifndef FSMGHOSTCONTROLLER_H_
#define FSMGHOSTCONTROLLER_H_

#include "Controller.h"
#include <random>
#include <chrono>
#include "FSM.h"

class FSMGhostStateMachine;

class FSMGhost: public Controller {
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
	std::shared_ptr<FSMGhostStateMachine> fsm;
public:
	FSMGhost(std::shared_ptr<Character> character);
	virtual ~FSMGhost();
	virtual Move getMove(const GameState& game)override;
};

#pragma region Transitions

class FSMGhostFrightenedTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public: 
	FSMGhostFrightenedTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class FSMGhostUnfrightTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next1;
	std::shared_ptr<FSMState> _next2;
public: 
	FSMGhostUnfrightTransition(
		std::shared_ptr<FSMState> next1, 
		std::shared_ptr<FSMState> next2, 
		std::shared_ptr<Character> character
	);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class FSMGhostTimeTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public:
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration> _start;
	FSMGhostTimeTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class FSMGhostChaseTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
	public:
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration> _start;
	FSMGhostChaseTransition(std::shared_ptr<FSMState> next,  std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class FSMGhostGuardTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
	public:
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration> _start;
	FSMGhostGuardTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class FSMGhostUnguardTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
	public:
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration> _start;
	FSMGhostUnguardTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};


#pragma endregion

#pragma region States

class FSMGhostFrightenedState : public FSMState {

public: 
	FSMGhostFrightenedState(std::shared_ptr<Character> character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~FSMGhostFrightenedState();
};

class FSMGhostChaseState : public FSMState {

public:
	FSMGhostChaseState(std::shared_ptr<Character> character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~FSMGhostChaseState();
};

class FSMGhostGuardState : public FSMState {
private:
	std::pair<int, int> target;
public: 
	FSMGhostGuardState(std::shared_ptr<Character> character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~FSMGhostGuardState();
};

#pragma endregion

class FSMGhostStateMachine : public FiniteStateMachine {

public:
	FSMGhostStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~FSMGhostStateMachine();
};

#endif