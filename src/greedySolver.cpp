#include "solver.hpp"

#include <algorithm>

GreedySolver::GreedySolver(Instance* inst) : Solver::Solver(inst) {
  name = "GreedySolver";
  desc = "Solver par méthode glouton (intelligent)";

  logn2("GreedySolver::GreedySolver BEGIN" + name + ": " + desc + " inst: " + inst->name);
  this->solution = new Solution(inst);

  // Code pour avoir un vecteur de stations trié selon les déficits:
  stations_triees = *(inst->stations); 
  std::sort(stations_triees.begin(), stations_triees.end(),
    [] (Station* s1, Station* s2) { 
      return s1->deficit() <= s2->deficit();
    });

  logn2("GreedySolver::GreedySolver END construit inst: " + inst->name);
}


GreedySolver::~GreedySolver()  {
    delete this->solution;
}

// Méthode principale de ce solver, principe :
// On utilise le vecteur trié de stations selon les déficits.
// On place à chaque fois la paire min-max de ce vecteur, et on les ajoutes
// à la suite dans une seule remorque. On remplis les autres circuits avec 
// juste une station
bool GreedySolver::solve() {


  // Not used in this function
  Options* args = Options::args;
  const string sinserter = args->station_inserter;
  const string rchooser = args->remorque_chooser;
  int itermax = args->itermax;
  // Par défaut (-1) on ne fait qu'une seule itération
  itermax =  itermax == -1 ? 1 : itermax;

  // Creation d'une nouvelle solution pour ne pas modifier la principale si echec
  Solution* sol = new Solution(inst);

  // Remplir la première remorque avec des couples min-max
  Circuit* circuit = sol->circuits->at(0);
  int begin_solo = 0; // position of the first alone station;
  int stations_to_fill_in_first = inst->stations->size() - inst->remorques->size() - 1;
  for (int i = 0; i < (stations_to_fill_in_first - stations_to_fill_in_first%2)/2; ++i)
  {
    logn5("GreedySolver::solve: ajout de la station " 
      + stations_triees.at(i)->name + " et " 
      + stations_triees.at(stations_triees.size()-1-i)->name
      + " à la remorque " + circuit->remorque->name);
    circuit->stations->push_back(stations_triees.at(i));
    circuit->stations->push_back(stations_triees.at(stations_triees.size()-1-i));
    ++begin_solo;
  }
  if (stations_to_fill_in_first%2 == 1) {
    circuit->stations->push_back(stations_triees.at(begin_solo));
    ++begin_solo;
  }

  // Remplir les autres remorques avec une unique station
  for (int circuit_id = 1; circuit_id < inst->remorques->size(); ++circuit_id)
  {
    circuit = sol->circuits->at(circuit_id);
    logn5("GreedySolver::solve: ajout de la station " 
          + stations_triees.at(begin_solo)->name +
          " à la remorque " + circuit->remorque->name);
    circuit->stations->push_back(stations_triees.at(begin_solo));
    ++begin_solo;
  }

  logn4("GreedySolver::solve: avant appel à sol->update");
  sol->update();
  this->found = true;
  this->solution = sol;
  logn3("GreedySolver::solve: END");
  return found;
}

//./
