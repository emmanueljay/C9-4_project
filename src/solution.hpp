#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include "common.hpp"
#include "circuit.hpp"
#include "options.hpp"

// CONVENTION : les objets de cette classe seront généralement appelés
// sol1, sol2, pour ne pas confodre avec solver (de clasee Solver !) ou
// s1 et s2 de classe  Site ou Station.
//
class Solution {
public:
    Instance* inst;
    vector<Circuit*>* circuits;

    // longueur total de la solution
    int length;

    // désequilibre total de la solution
    int desequilibre;

    // // coût total de la solution (length des circuits + pénalité)
    // int cost;

    Solution(Instance* inst);

    // Attention : ceci n'est pas le constructeur par copie car il prend un
    // **pointeur** en parametre
    Solution(const Solution*);

    // Le constructeur par copie officiel c++
    /// Solution(const Solution&);

    void copy(Solution* other);

    virtual ~Solution();

    // vide chaque circuit (dont les listes de stations), et réinitialise
    // les attributs dérivés (deseuilibre, ...)
    void clear();


    // met à jour tous les circuits, et calcule les attributs dérivés de la
    // solution (desequilibres, charge_init, ...)
    void update();

    // vrai ssi la solution est acceptable (e.g. les remorques desservent au
    // moins une station, ...; toutes les station sont desservies, ...)
    // bool is_valid();

    // Retourne la chaine de sortie de la solutin au format standard
    string to_s();
    string to_s_long();

    // Construit un nom de fichier de la forme
    //    velib_n_dd_mmmm.sol
    // avec
    //    d : numéro d'instance
    //    nn : desequilibre global
    //    mmmm : longueur de la solution
    //
    string get_tmp_filename();

    // Exploite la solution en fonction des options de l'appli (classe Args)
    /// static void main_print_solution(Solution* solution, Args* args);
    static void main_print_solution(Solution* solution, Options* args);

    // Retourne un coût pour povoir comparer deux solutions entre elles
    // C'est une combinaison lexicogarfique des critères desequilibre et lengh
    inline int get_cost() {
        // return 1000*this->desequilibre + this->length;
        return 1000000*this->desequilibre + this->length;
    }
    inline string get_cost_string() {
        return U::to_s(this->desequilibre) + "-" + U::to_s(this->length);
    }
};
#endif
//./
