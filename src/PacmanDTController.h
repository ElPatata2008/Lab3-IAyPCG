
#ifndef PACMANDTCONTROLLER_H_
#define PACMANDTCONTROLLER_H_
#include "Controller.h"

class PacmanDTController : public Controller {
    PacmanDTController(std::shared_ptr<Character> character);
    virtual ~PacmanDTController();
    virtual Move getMove(const GameState& game) override;
};

#endif