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
// 
bool GreedySolver::solve() {

  // Code pour avoir un vecteur de stations trié :
  std::vector<Station*> stations_triees = *(inst->stations); 
  std::sort(stations_triees.begin(), stations_triees.end(),
    [] (Station* s1, Station* s2) { 
      return s1->deficit() <= s2->deficit();
    });

  for (std::vector<Station*>::iterator i = stations_triees.begin(); i != stations_triees.end(); ++i)
    std::cout << (*i)->deficit() << " -- "; 
  std::cout << std::endl;


  found = false;
  return found;
}

//./
