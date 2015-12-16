#ifndef SOLVER_H
#define SOLVER_H

#include "common.hpp"
#include "instance.hpp"
#include "solution.hpp"

class Solver {
public:

    // nom court du solver e.g "S_1_glouton"
    string name;
    // description en une ligne
    // e.g "Par itération : un simplex + un mip + un seul sous tour"
    string desc;

    // pointeur sur l'instance du problème
    Instance* inst;

    // un boolean parmettent de connaitre l'état de la résolution en fin de calcul
    // TODO : différencier les états "optimum trouvé" dans état "interrompu avant
    // optimum"
    // true : le solveur à terminé normalement sa fonction qui dépend de la
    //        méthode appelée, en général c'est que l'optimum a été trouvé
    // false : une interruption à été déclenchée avant la fin de la résolution
    // TODO TRANSFORMER EN ATTRIBUT state :
    // - found : solution trouvée fin normale du solver
    // - notfound : solution non trouvée (car interuption avant la fin, ...)
    // - optimum : solution optimale trouvée (version branch and bound ou PPC)
    //
    // enum State {
    //     INIT,
    //     RUNING,
    //     INTERRUPTED,
    //     FOUND,
    //     OPTIMUM
    // }
    // State state = INIT;
    bool found;

    Solution* solution;

    Solver(Instance* Instance);
    virtual ~Solver();

    // La principal méthode à redéfinir par les classes filles
    //
    virtual bool solve();
    // virtual State solve();

    virtual Solution* get_solution()  {return solution ; };

};

////////////////////////////////////////////////////////////////////////
// Déclaration des solveur spécialisés
//
class StupidSolver : public Solver {
public:
    StupidSolver(Instance* Instance);
    virtual ~StupidSolver();
    virtual bool solve();
    Solution* solution;
    Solution* get_solution() {return this->solution;};
};

class CarloSolver : public Solver {
public:
    CarloSolver(Instance* Instance);
    virtual ~CarloSolver();
    virtual bool solve();
    virtual Solution* apply_one_greedy(Solution* currentsol);
    Solution* get_solution() {return this->solution;};
};

class GreedySolver : public Solver {
public:
    GreedySolver(Instance* Instance);
    virtual ~GreedySolver();
    virtual bool solve();
    Solution* solution;
    std::vector<Station*> stations_triees;
    std::vector<Remorque*> remorques_triees;
    Solution* get_solution() {return this->solution;};

};

class AnnealingSolver : public Solver {
public:
    AnnealingSolver(Instance* Instance);
    virtual ~AnnealingSolver();
    virtual bool solve();
    Solution* get_solution() {return this->solution;};
};

class TabuSolver : public Solver {
public:
    TabuSolver(Instance* Instance);
    virtual ~TabuSolver();
    virtual bool solve();
    Solution* get_solution() {return this->solution;};
};

#endif

