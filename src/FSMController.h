/*
 * FSMController.h
 *
 *  Created on: Apr 23, 2018
 *      Author: nbarriga
 */

#ifndef FSMCONTROLLER_H_
#define FSMCONTROLLER_H_

#include "Controller.h"
#include <random>
#include <chrono>
#include "FSM.h"

class ExampleStateMachine;

class FSMController: public Controller {
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
	std::shared_ptr<ExampleStateMachine> fsm;
public:
	FSMController(std::shared_ptr<Character> character);
	virtual ~FSMController();
	virtual Move getMove(const GameState& game)override;
};

class PillTransition:public FSMTransition{
	int last;
	std::shared_ptr<FSMState> _next;
public:
	PillTransition(std::shared_ptr<FSMState> next);
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
};

class NonFrightenedTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
	std::shared_ptr<FSMState> _next1;
	std::shared_ptr<FSMState> _next2;
public:
	NonFrightenedTransition(std::shared_ptr<FSMState> next1, std::shared_ptr<FSMState> next2, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class FrightenedTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public:
	FrightenedTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class ChaseTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
public:
	ChaseTransition(std::shared_ptr<FSMState> next,  std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class ScatterTransition : public FSMTransition {
	std::shared_ptr<Character> _character;
	std::shared_ptr<FSMState> _next;
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration> _start = std::chrono::high_resolution_clock::now();
public:
	bool change = false;
	ScatterTransition(std::shared_ptr<FSMState> next, std::shared_ptr<Character> character);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class ChaseState:public FSMState{
	
public:
	ChaseState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~ChaseState();
};

class ScatterState:public FSMState{
	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration> _start;
public:
	ScatterState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~ScatterState();
};

class FrightenedState : public FSMState {
	
public:
	FrightenedState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~FrightenedState();
};

class ExampleStateMachine: public FiniteStateMachine{

public:
	ExampleStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~ExampleStateMachine();

};
#endif /* FSMCONTROLLER_H_ */
