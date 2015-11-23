#include "solver.hpp"


GreedySolver::GreedySolver(Instance* inst) : Solver::Solver(inst) {
    name = "GreedySolver";
    desc = "Solver par méthode glouton (intelligent)";
    cerr << "\nGreedySolver non implémenté : AU BOULOT !" << endl;
    logn1(name + ": " + desc + " inst: " + inst->name);
    exit(1);
}
GreedySolver::~GreedySolver()  {
    // TODO
}
// Méthode principale de ce solver, principe :
//
bool GreedySolver::solve() {
    found = false;
    return found;
}

//./
