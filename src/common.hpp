#ifndef COMMON_H
#define COMMON_H

////////////////////////////////////////////////////////////////////////////////
// Quelques include et déclaration pour tous le projet
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// #include <getopt.h>
#include <time.h>

#include <cassert>




#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <algorithm>
// #include <deque>
// #include <functional>
// #include <iterator>
#include <vector>
#include <list>
#include <map>
// #include <memory>
// #include <numeric>
// #include <set>
// #include <stack>
// #include <utility>

using namespace std;

// extern "C" {
// #include "glpk.h"
// }

////////////////////////////////////////////////////////////////////////////////
// Fichiers spécifique à l'application

// Quelques defines qui pourront être transformées en options
#define EPS 1.0e-10

#include "options.hpp"

// Quelques classe générique du projet
#include "util.hpp"
#include "logger.hpp"

// Quelques prédéclaration de classe
class Instance;
class Site;
class Remorque;
class Station;
class Arc;
class Circuit;
class Solution;
class Solver;
class StupidSolver;
class CarloSolver;
class GreedySolver;
class AnnealingSolver;
class TabuSolver;

// Quelques typedef pour e sdéclaratins fréquente
typedef vector<Station*> Stations;
typedef vector<Remorque*> Remorques;
// typedef pair<Remorque*,int> Service; // non testé : association <remorque,dépot>

// TODO : NE PAS INCLURE ICI LES CLASSES SPECIFIQUES.
// IL FAUT LES INCLURE DANS LES FICHIERS QUI LES UTILISENT VRAIMENT.

#endif /* COMMON_H */
//./
