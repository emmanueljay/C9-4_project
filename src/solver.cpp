#include "solver.hpp"

Solver::Solver(Instance* inst) {

    name = "S_0_abstract";
    desc = "Par itération : un simplex + un mip + un seul sous tour";

    this->inst = inst;
    found = false;
}
Solver::~Solver() {
    // todo;
}

// Méthode principale de ce solver, principe :
//
bool Solver::solve() {
    found = false;
    return found;
}
// ./

