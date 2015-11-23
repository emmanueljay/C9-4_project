#include "solver.hpp"

TabuSolver::TabuSolver(Instance* inst) : Solver::Solver(inst) {
    name = "TabuSolver";
    desc = "Solver par méthode tabou";
    cerr << "TabuSolver non implémenté !" << endl;
    exit(1);
}
TabuSolver::~TabuSolver()  {
}
// Méthode principale de ce solver, principe :
//
bool TabuSolver::solve() {
    found = false;
    return found;
}
//./

