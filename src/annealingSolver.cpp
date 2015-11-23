#include "solver.hpp"

AnnealingSolver::AnnealingSolver(Instance* inst) : Solver::Solver(inst) {
    name = "AnnealingSolver";
    desc = "Solver par recuit simulé";
    cerr << "\nAnnealingSolver non implémenté : AU BOULOT !" << endl;
    exit(1);
}
AnnealingSolver::~AnnealingSolver()  {
    // TODO
}
// Méthode principale de ce solver, principe :
//
bool AnnealingSolver::solve() {
    found = false;
    return found;
}
//./
