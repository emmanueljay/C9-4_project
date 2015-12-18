#include "solver.hpp"

#include <algorithm>

GreedySolver::GreedySolver(Instance* inst) : Solver::Solver(inst) {
  name = "GreedySolver";
  desc = "Solver par méthode glouton (intelligent)";

  logn2("GreedySolver::GreedySolver BEGIN" + name + ": " + desc + " inst: " + inst->name);
  this->solution = new Solution(inst);

  logn3("GreedySolver::GreedySolver Sorting Vector for stations");
  // Code pour avoir un vecteur de stations trié selon les déficits:
  stations_triees = std::vector<Station*>(inst->stations->begin(),inst->stations->end()); 
  std::sort(stations_triees.begin(), stations_triees.end(),
    [] (Station* s1, Station* s2) { 
      return s1->deficit() > s2->deficit();
    });

  logn3("GreedySolver::GreedySolver Sorting Vector for trucks");
  // Code pour avoir un vecteur de stations trié selon les déficits:
  remorques_triees = std::vector<Remorque*>(inst->remorques->begin(),inst->remorques->end()); 
  std::sort(remorques_triees.begin(), remorques_triees.end(),
    [] (Remorque* r1, Remorque* r2) { 
      return r1->capa > r2->capa;
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

  for (std::vector<Remorque*>::iterator i = remorques_triees.begin(); i != remorques_triees.end(); i++ )
    std::cout << (*i)->capa << " -- ";
  std::cout << std::endl;


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
  Circuit* circuit = sol->circuits->at(remorques_triees.at(0)->id);
  int begin_solo = 0; // position of the first alone station;
  int stations_to_fill_in_first = inst->stations->size() - inst->remorques->size() + 1;
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
  for (int circuit_id = 0; circuit_id < inst->remorques->size(); ++circuit_id)
  {
    if (circuit_id != remorques_triees.at(0)->id){
      circuit = sol->circuits->at(circuit_id);
      logn5("GreedySolver::solve: ajout de la station " 
            + stations_triees.at(begin_solo)->name +
            " à la remorque " + circuit->remorque->name);
      circuit->stations->push_back(stations_triees.at(begin_solo));
      ++begin_solo;
    }  
  }

  logn4("GreedySolver::solve: avant appel à sol->update");
  sol->update();
  this->found = true;
  this->solution = sol;
  logn3("GreedySolver::solve: END");
  return found;
}

// Deuxième méthode de ce solver, principe :
// On utilise le vecteur trié de stations selon les déficits.
// On place à chaque fois la paire min-max de ce vecteur, et on les ajoute
// au mieux dans une seule remorque. On remplis les autres circuits avec 
// juste une station
bool GreedySolver::solve_insertbest() {

  for (std::vector<Remorque*>::iterator i = remorques_triees.begin(); i != remorques_triees.end(); i++ )
    std::cout << (*i)->capa << " -- ";
  std::cout << std::endl;

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
  Circuit* circuit = sol->circuits->at(remorques_triees.at(0)->id);
  int begin_solo = 0; // position of the first alone station;
  int stations_to_fill_in_first = inst->stations->size() - inst->remorques->size() + 1;
  for (int i = 0; i < (stations_to_fill_in_first - stations_to_fill_in_first%2)/2; ++i)
  {
    logn5("GreedySolver::solve_insertbest: ajout de la station " 
      + stations_triees.at(i)->name + " et " 
      + stations_triees.at(stations_triees.size()-1-i)->name
      + " à la remorque " + circuit->remorque->name);
    circuit->insert_best(stations_triees.at(i));
    circuit->insert_best(stations_triees.at(stations_triees.size()-1-i));
    ++begin_solo;
  }
  if (stations_to_fill_in_first%2 == 1) {
    circuit->insert_best(stations_triees.at(begin_solo));
    ++begin_solo;
  }

  // Remplir les autres remorques avec une unique station
  for (int circuit_id = 0; circuit_id < inst->remorques->size(); ++circuit_id)
  {
    if (circuit_id != remorques_triees.at(0)->id){
      circuit = sol->circuits->at(circuit_id);
      logn5("GreedySolver::solve_insertbest: ajout de la station " 
            + stations_triees.at(begin_solo)->name +
            " à la remorque " + circuit->remorque->name);
      circuit->stations->push_back(stations_triees.at(begin_solo));
      ++begin_solo;
    }  
  }

  logn4("GreedySolver::solve_insertbest: avant appel à sol->update");
  sol->update();
  this->found = true;
  this->solution = sol;
  logn3("GreedySolver::solve_insertbest: END");
  return found;
}

// Function smart d'insertion
// Function that try to insert normaly,
// then try to insert best,
// finaly changes the truck, and insert best. 
void smart_insert_in_trucks(Circuit* fst_circuit, Circuit* snd_circuit, Station* station) {
  logn5("GreedySolver::smart_insert_in_trucks: Trying to insert station : " + std::to_string(station->id));

  // Fisrt : We try to push it at the end of the first circuit
  fst_circuit->stations->push_back(station);
  fst_circuit->update();

  // Getting the equilibrium value. If == 0, we get out
  if(fst_circuit->desequilibre==0){
    logn5("GreedySolver::smart_insert_in_trucks: Inserted normaly in biggest truck");
    return;
  }
  else {
    // We remove the station at the end
    fst_circuit->stations->pop_back();
  }

  // Trying to do an insert_best
  fst_circuit->insert_best(station);
  fst_circuit->update();

  // Getting the equilibrium value. If == 0, we get out
  if(fst_circuit->desequilibre==0) {
    logn5("GreedySolver::smart_insert_in_trucks: Inserted best in biggest truck");
    return;
  }
  else {
    // We remove the station previously inserted
    fst_circuit->stations->erase(std::find(fst_circuit->stations->begin(), fst_circuit->stations->end(), station));
  } 

  // Insertion best in the second truck
  snd_circuit->insert_best(station);
  logn5("GreedySolver::smart_insert_in_trucks: Inserted best in second biggest truck");
  return;
}  


// Troisème méthode de ce solver, principe :
// 
// c'est tu le mets au bout du circuit de la premiere reporque
// si ca marche g onext station
// sinon tu l'enleves du bout et tu tentes l'insert best
// si ca marche go next station
// sinon tu la mets dans la deuxieme remorque
// (le deuxieme la plus grosse)
bool GreedySolver::solve_use_other_truck() {

  logn3("GreedySolver::solve_using_other_truck: BEGIN");

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
  Circuit* circuit_r0 = sol->circuits->at(remorques_triees.at(0)->id);
  Circuit* circuit_r1;
  if (inst->remorques->size() == 1)
    circuit_r1 = circuit_r0;
  else 
    circuit_r1 = sol->circuits->at(remorques_triees.at(1)->id);
  
  int begin_solo = 0; // position of the first alone station;
  int stations_to_fill_in_first = inst->stations->size() - inst->remorques->size() + 1;
  for (int i = 0; i < (stations_to_fill_in_first - stations_to_fill_in_first%2)/2; ++i)
  {    
    // Insertion at the end
    
    smart_insert_in_trucks(circuit_r0, circuit_r1, stations_triees.at(i));
    smart_insert_in_trucks(circuit_r0, circuit_r1, stations_triees.at(stations_triees.size()-1-i));

    ++begin_solo;
  }
  if (stations_to_fill_in_first%2 == 1) {
    smart_insert_in_trucks(circuit_r0, circuit_r1, stations_triees.at(begin_solo));
    ++begin_solo;
  }

  // Remplir les autres remorques avec une unique station
  Circuit* circuit;
  for (int circuit_id = 0; circuit_id < inst->remorques->size(); ++circuit_id)
  {
    if (circuit_id != remorques_triees.at(0)->id){
      circuit = sol->circuits->at(circuit_id);
      logn5("GreedySolver::solve_insertbest: ajout de la station " 
            + stations_triees.at(begin_solo)->name +
            " à la remorque " + circuit->remorque->name);
      circuit->insert_best(stations_triees.at(begin_solo));
      ++begin_solo;
    }  
  }

  sol->update();
  this->found = true;
  this->solution = sol;
  logn3("GreedySolver::solve_use_other_truck: END");
  return found;
}







