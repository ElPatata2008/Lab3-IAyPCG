#ifndef PACMANCONTROLLER_H_
#define PACMANCONTROLLER_H_

#include "Controller.h"
#include <random>
#include <chrono>
#include "FSM.h"

class PacmanStateMachine;

class PacmanController: public Controller {
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
	std::shared_ptr<PacmanStateMachine> fsm;
public:
	PacmanController(std::shared_ptr<Character> character);
	virtual ~PacmanController();
	virtual Move getMove(const GameState& game)override;
};

#pragma region Transitions

class LookForPowerTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public:
	LookForPowerTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class PowerOutOfRangeTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public:
	PowerOutOfRangeTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class LookForPillsTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public:
	LookForPillsTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class AttackGhostsTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public:
	AttackGhostsTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class PowerDownTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public:
	PowerDownTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class RunFromGhostsTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public:
	RunFromGhostsTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

#pragma endregion

#pragma region States

class SearchPowerState : public FSMState {
public: 
	SearchPowerState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~SearchPowerState();
};

class ScavengeState : public FSMState {
public: 
	ScavengeState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~ScavengeState();
};

class EscapeState : public FSMState {
public: 
	EscapeState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~EscapeState();
};

class AttackState : public FSMState {
public: 
	AttackState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~AttackState();
};

#pragma endregion

class PacmanStateMachine : public FiniteStateMachine {
public:
	PacmanStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~PacmanStateMachine();
};

#endif