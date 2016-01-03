#include "common.hpp"

#include "instance.hpp"
#include "solver.hpp"

#include "options.hpp"

// je dois définir cette variable statique en dehors de la classe Options
Options* Options::args = NULL;

// TODO : Remplacer classe Solver par un interface purement abstraite
//        et réorganiser ce fichier
//
int main(int argc, char *argv[]) {

    // Analyse et extraction des arguments de la ligne de commande
    Options::args =  new Options(argc, argv);
    Options* args = Options::args;


    // Exemple d'affichage de quelques options
    cout << "args->solver=" << args->solver << endl;
    cout << "args->seed=" << args->seed << endl;
    cout << "args->filename=" << args->filename << endl;
    cout << "args->outfilename=" << args->outfilename << endl;
    // exit(0);

    if (args->solver == 0) {
        logn1("s=0 => Test construction d'instance new_velib_mini\n");
        Instance* inst = Instance::new_velib_mini();
        cout << inst->to_s();
        return 0;
    }

    // La suite nécessite la lecture d'une instance passée en paramètre
    //
    Instance* inst;
    inst = new Instance(args->filename, args);
    logn1("Instance construite\n");
    if (log5()) {
        cout << "Regénération de l'instance\n";
        cout << inst->to_s();
    }

    // Démarrage d'un chronomètre
    time_t start_time;
    time(&start_time);

    // Dans la suite, le comportement de la résolution dépend du choix du
    // solveur passé en paramètre.
    if (args->solver == 1) {;
        logn1("Solver 1 : lecture, affichage et enregistrement d'une instance");
        inst->print_instance();
        inst->print_instance("mini_rand.dat");
        logn1("Solver 1 : fin de test instance");
    } else if (args->solver == 2) {;
        logn1("Solver 2 : glouton (StupidSolver)");
        StupidSolver* solver = new StupidSolver(inst);
        solver->solve();
        if (solver->found) {
            cout << "Solver : solution trouvée de coût "
                 << solver->solution->get_cost() << endl;
            // cout << "Solver 2 : solution trouvée de coût "<< endl;
            Solution* sol = solver->get_solution();
            Solution::main_print_solution(sol, args);
            cout << "solution trouvée de coût "
                 << solver->solution->get_cost_string()
                 << " (soit " << solver->solution->get_cost() << ")"
                 << endl;
        } else {
            cout << "StupidSolver : pas de solution" << endl;
        }
        delete solver;
        logn1("Solver 2 : fin de StupidSolver");
    } else if (args->solver == 3) {;
        logn1("Solver 3 : montecarlo (CarloSolver)");
        CarloSolver* solver = new CarloSolver(inst);
        solver->solve();
        if (solver->found) {
            cout << "Solver : solution trouvée de coût "
                 << solver->solution->get_cost() << endl;
            Solution* sol = solver->get_solution();
            Solution::main_print_solution(sol, args);
            cout << "solution trouvée de coût "
                 << solver->solution->get_cost_string()
                 << " (soit " << solver->solution->get_cost() << ")"
                 << endl;
        } else {
            cout << "Solver : pas de solution "  << endl;
        }
        delete solver;
        logn1("Solver 3 : fin de CarloSolver");
    } else if (args->solver == 4) {;
        logn1("Solver 4 : glouton (GreedySolver)");
        GreedySolver* solver = new GreedySolver(inst);
        int greedy_variant = args->greedy_variant; // Using Itermax to store information
        if (greedy_variant == 100) greedy_variant = 2;
        switch (greedy_variant){
            case 0 : {cout << "Using solver 0" << endl; solver->solve(); break;}
            case 1 : {cout << "Using solver 1" << endl; solver->solve_insertbest(); break;}
            case 2 : {cout << "Using solver 2" << endl; solver->solve_use_other_truck(); break;}
            cout << "ERROR : WRONG PARAM (ITER MAX) TO WHICH GREEDY SOLVER USE (0 to 2)" << endl;
        }
        if (solver->found) {
            cout << "Solver : solution trouvée de coût "
                 << solver->solution->get_cost() << endl;
            Solution* sol = solver->get_solution();
            Solution::main_print_solution(sol, args);
            cout << "solution trouvée de coût "
                 << solver->solution->get_cost_string()
                 << " (soit " << solver->solution->get_cost() << ")"
                 << endl;
        } else {
            cout << "Solver : pas de solution "  << endl;
        }
        delete solver;
        logn1("Solver 4 : fin de GreedySolver");
    } else if (args->solver == 5) {;
        logn1("Solver 5 : Recuit simulé (AnnealingSolver)");
        // Le recuit simulé
        /// s = new SolverRecuit(b);
        AnnealingSolver* solver = new AnnealingSolver(inst);
        solver->solve();
        if (solver->found) {
            cout << "Solver : solution trouvée de coût "
                 << solver->solution->get_cost() << endl;
            Solution* sol = solver->get_solution();
            Solution::main_print_solution(sol, args);
        } else {
            cout << "Solver : pas de solution "  << endl;
        }
        delete solver;
        logn1("Solver 5 : fin de AnnealingSolver");
    } else if (args->solver == 6) {;
        logn1("Solver 6 : méthode tabou (TabuSolver)");
        TabuSolver* solver = new TabuSolver(inst);
        solver->solve();
        if (solver->found) {
            cout << "Solver : solution trouvée de coût "
                 << solver->solution->get_cost() << endl;
            Solution* sol = solver->get_solution();
            Solution::main_print_solution(sol, args);
        } else {
            cout << "Solver : pas de solution "  << endl;
        }
        delete solver;
        logn1("Solver 5 : fin de TabuSolver");
    } else {
        cerr << "Numéro de solveur non reconnue : " << args->solver << "\n";
        exit(1);
    }

    // Arrêt et exploitation du chronomètre
    time_t end_time;
    time(&end_time);
    double diff = difftime(end_time, start_time);
    printf("Temps de calcul:\t %.1fs\n", diff);
    // cout << "Temps de calcul:\t" << diff << "s" << endl << endl;


    // Nettoyage des objets alloués dans ce main()
    // delete solver;
    delete inst;
    delete args;
    return 0;
}
//./
