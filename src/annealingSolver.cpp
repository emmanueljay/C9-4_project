#include "solver.hpp"

#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include <cmath>      /* exp */


AnnealingSolver::AnnealingSolver(Instance* inst) : Solver::Solver(inst) {
    name = "AnnealingSolver";
    desc = "Solver par recuit simulé";
    logn2("AnnealingSolver::AnnealingSolver BEGIN" + name + ": " + desc + " inst: " + inst->name);
    this->solution = new Solution(inst);
    logn2("AnnealingSolver::AnnealingSolver END construit inst: " + inst->name);
}
AnnealingSolver::~AnnealingSolver()  {
    delete this->solution;
}

// Principe : On prend les stations dans l'ordre de l'instance, puis
// on les affecte a chaque remorque à tour de rôle.
void AnnealingSolver::get_initial_solution(Solution* sol) {
  int remorque_id = -1;
  for (unsigned j = 0; j < inst->stations->size(); j++) {
      // on extrait la prochaine station à visiter
      Station* station = inst->stations->at(j);
      // sélection des remorques à tour de rôle
      remorque_id = (remorque_id + 1) % inst->remorques->size();
      // on extrait le circuit associé à la remorque sélectionnée
      Circuit* circuit = sol->circuits->at(remorque_id);
      // on ajoute la station en fin de ce circuit
      logn5("AnnealingSolver::solve: ajout de la station " + station->name +
            " à la remorque " + circuit->remorque->name);
      circuit->stations->push_back(station);
      // update inutile ici car n'a pas (encore) besoin de mesurer la
      // stupidité de cette insertion !
  } 
  sol->update();
}

double AnnealingSolver::get_energy(Solution* sol, int version) {
  double energy = 0.0; // Valeur de l'energie

  switch (version) {
    case 0 : {
      // First version of the implementation : using the cost of the solution
      energy = sol->get_cost();
      break;
    }
    case 1 : {
      logn0("Not emplemented");
      break;
    }
  }
  return energy;
}

double AnnealingSolver::get_next_temperature(double old_temp, double lambda, int version) {
  double temperature = 0.0;

  switch (version) {
    case 0 : {
      // First version of the implementation : using Tnext = lambda * Told
      temperature = old_temp * lambda;
      break;
    }
    case 1 : {
      logn0("Not emplemented");
      break;
    }
  }
  return temperature;
}

/**
 * Function that return a neighbour of the function randomly
 * if we have nb_sta stations, we pick a position in 0,nb_sta-1 and we select the 
 * corresponding station in circuit
 */
void AnnealingSolver::get_neighbour(Solution* sol, Solution* old_sol) {
  // Clearing the new sol and copying the old solution into the new one.
  sol->clear();
  sol->copy(old_sol);

  // Chosing a station randomly, to be removed.
  Station* station;
  int station_id = rand() % inst->nb_stations;
  logn4("We selected the station at position : " + to_string(station_id));

  for (Circuit* circuit : *(sol->circuits)) {
    list<Station*>* stations =  circuit->stations;
    if (station_id < stations->size()) {
      // Finding station
      std::list<Station*>::iterator it = stations->begin();
      std::advance(it, station_id);
      station = *(it);

      // Removing station from circuit
      stations->erase(it);

      break;
    }
    else 
      station_id -= stations->size();
  }

  // We remove the station from the circuit

  // Chosing a cuircuit where we will insert_best the solution to.
  int remorque_id = rand() % inst->nb_remorques;
  logn4("We selected the remorque with id : " + to_string(remorque_id));

  Circuit* circuit = sol->circuits->at(remorque_id);
  circuit->insert_best(station); 

  sol->update();
  return;
}

/**
 * IMPLEMENTATION D'UN RECUIT SIMULÉ 
 *
 * Voisinage : On enlève une station d'une remorque, et on l'insert_best 
 * (ou juste insert) dans une des remorque (sa remorque y compris, sa place
 * peut n'être plus optimale avec d'autres insertions)
 *
 * Energie : 
 *  - Version 0 : Cout de la solution
 *
 * Temperature : 
 *  - Version 0 : Décroissance de la température en T(n+1) = lambda * T(n)
 *   avec lambda = 0.99
 *
 * Solution initiale : La solution la plus bête possible, pour ne pas ensuite
 * être gêné par une solution qui serait trop bonne.
 * 
 * Algorithme :
 *  s := s0     // solution
 *  e := E(s)   // energie
 *  k := 0      // Iteration
 *  tant que k < kmax et e > emax
 *    sn := voisin(s)
 *    en := E(sn)
 *    si en < e ou aléatoire() < P(en - e, temp(k/kmax)) alors
 *      s := sn; e := en
 *    k := k + 1
 *  retourne s
 * 
 * @return Si une solution a pu être trouvée
 */
bool AnnealingSolver::solve() {
  if (log2()) {
      cout << "\n---AnnealingSolver::solve START instance: "
           << inst->name << " de taille "
           << inst->stations->size() << "\n";
  }

  // Seed initialisation
  srand (time(NULL));
  // ((double) rand() / (RAND_MAX)) // double btwn 0 and 1

  // Arguments
  Options* args = Options::args;
  const string sinserter = args->station_inserter;
  const string rchooser = args->remorque_chooser;
  int itermax = args->itermax;
  // Par défaut (-1) on ne fait qu'une seule itération
  itermax =  itermax == -1 ? 1 : itermax;

  // Recuit informations
  int temperature = 10000;   // Initial temperature
  double energy_max = 0;  // Energy maximum of acceptation
  double energy;          // Energy
  int k = 0;              // Iteration
  Solution* neighbour = new Solution(inst); // Intermediate solution

  // INITIAL SOLUTION (Stupid)
  Solution* sol = new Solution(inst);
  get_initial_solution(sol);
  energy = get_energy(sol);
  logn2("AnnealingSolver::Cost of the initial solution = "+sol->get_cost_string());
  logn2("AnnealingSolver::Energy of the initial solution = "+to_string(energy));

  Solution* best_sol = new Solution(inst);
  best_sol->copy(sol);

  // RECUIT
  while (k < itermax && energy > energy_max) {
    logn3("AnnealingSolver:: Iteration number "+to_string(k));

    // Updating Temperature
    if (k % 10 == 0)
      temperature = get_next_temperature(temperature);
    logn3("AnnealingSolver:: Temperature value "+to_string(temperature));

    // Getting neighbour
    get_neighbour(neighbour,sol);
    double energy_neighbour = get_energy(neighbour);
    logn3("AnnealingSolver:: Energy of neighbour = "+to_string(energy_neighbour));

    // Choosing the solution
    if (energy_neighbour < energy ||
        ((double) rand() / (RAND_MAX)) < exp(energy_neighbour - energy/(temperature+0.0001))) 
    {
      logn3("AnnealingSolver:: New solution found, energy = "+to_string(energy_neighbour));

      sol->clear();
      sol->copy(neighbour);

      if (sol->get_cost() < best_sol->get_cost()) {
        best_sol->clear();
        best_sol->copy(sol);
      }

      energy = energy_neighbour;
    }

    // Updating iteration
    ++k;
  }

  this->found = true;
  this->solution = best_sol;
  logn2("StupidSolver::solve: END");
  return found;
}
