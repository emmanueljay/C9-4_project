#include "solver.hpp"

StupidSolver::StupidSolver(Instance* inst) : Solver::Solver(inst) {
    name = "StupidSolver";
    desc = "Résolution par algorithme glouton stupide";
    logn2("StupidSolver::StupidSolver BEGIN" + name + ": " + desc + " inst: " + inst->name);
    this->solution = new Solution(inst);
    logn2("StupidSolver::StupidSolver END construit inst: " + inst->name);
}
StupidSolver::~StupidSolver()  {
    delete this->solution;
}

// Méthode principale de ce solver, principe :
//
//
bool StupidSolver::solve() {
    if (log3()) {
        cout << "\n---StupidSolver::solve START instance: "
             << inst->name << " de taille "
             << inst->stations->size() << "\n";
    }

    Options* args = Options::args;
    const string sinserter = args->station_inserter;
    const string rchooser = args->remorque_chooser;
    int itermax = args->itermax;
    // Par défaut (-1) on ne fait qu'une seule itération
    itermax =  itermax == -1 ? 1 : itermax;


    // Principe : On prend les stations dans l'ordre de l'instance, puis
    // on les affecte a chaque remorque à tour de rôle.
    //
    Solution* sol = new Solution(inst);
    int remorque_id = -1;
    for (unsigned j = 0; j < inst->stations->size(); j++) {
        // on extrait la prochaine station à visiter
        Station* station = inst->stations->at(j);
        // sélection des remorques à tour de rôle
        remorque_id = (remorque_id + 1) % inst->remorques->size();
        // on extrait le circuit associé à la remorque sélectionnée
        Circuit* circuit = sol->circuits->at(remorque_id);
        // on ajoute la station en fin de ce circuit
        logn5("StupidSolver::solve: ajout de la station " + station->name +
              " à la remorque " + circuit->remorque->name);
        circuit->stations->push_back(station);
        // update inutile ici car n'a pas (encore) besoin de mesurer la
        // stupidité de cette insertion !
    }
    logn4("StupidSolver::solve: avant appel à sol->update");
    sol->update();
    this->found = true;
    this->solution = sol;
    logn3("StupidSolver::solve: END");
    return found;
}
//./

