#include "solution.hpp"

Solution::Solution(Instance* inst) {
    this->inst = inst;
    this->circuits = new vector<Circuit*>(this->inst->remorques->size());
    // for (vector<Remorque*>::iterator it = inst->remorques->begin(); it != inst->remorques->end(); it++) {
    //     Remorque* remorque = *it;
    // }
    // for (auto it = inst->remorques->begin(); it != inst->remorques->end(); it++) {
    //     Remorque* remorque = *it;
    // }
    for (unsigned i = 0; i < inst->remorques->size(); i++) {
        Remorque* remorque = inst->remorques->at(i);
        Circuit* circ = new Circuit(inst, remorque);
        this->circuits->at(i) = circ; // ne marche que si circuits prédimentionné
        // this->circuits->push_back(circ);
        // this->circuits->at(i) = new Circuit(inst, inst->remorques->at(i));
    }

    // La solution est invalide à la construction : on invente un coût
    // "infini" grâce à un longueur arbitrairement élevée !
    // this->length = 0;
    this->length = 999999999;
    this->desequilibre = 0;

}
Solution::Solution(const Solution* other)  {
    this->inst = other->inst;
    this->length = other->length;
    this->desequilibre = other->desequilibre;
    this->circuits = new vector<Circuit*>(this->inst->remorques->size());
    for (unsigned i = 0; i < this->inst->remorques->size(); i++) {
        Circuit circ = other->circuits->at(i);
        this->circuits->at(i) = new Circuit(circ);
    }
}

Solution::~Solution() {

    for (unsigned i = 0; i < inst->remorques->size(); i++) {
        delete this->circuits->at(i);
        //// cout << "Solution::~Solution pout i=" << i;
    }
    delete this->circuits;
}

void Solution::copy(Solution* other) {
    this->inst = other->inst;
    this->length = other->length;
    this->desequilibre = other->desequilibre;
    // ASSERT this->circuits->size() == other->circuits->size()
    for (unsigned i = 0; i < this->circuits->size(); i++) {
        this->circuits->at(i)->copy(other->circuits->at(i));
    }
}
void Solution::clear() {
    length = 999999999;
    this->desequilibre = 0;
    for (unsigned i = 0; i < circuits->size(); i++) {
        circuits->at(i)->clear();
    }
}
void Solution::update() {
    logn4("Solution::update BEGIN");
    // Reset des attributs dérivés de la solution
    length = 0;
    desequilibre = 0;
    // cerr<<"FIN"; exit(1);
    // Mise à jour de chaque circuit, puis des attributs de la solution
    // length, desequilibre)
    for (unsigned i = 0; i < circuits->size(); i++) {
        Circuit* circuit = circuits->at(i);
        circuit->update();
        logn5("Solution::update: remorque=" + circuit->remorque->name);
        logn5("  cost=" + U::to_s(circuit->desequilibre) + "--"
                        + U::to_s(circuit->length));
        this->length += circuit->length;
        this->desequilibre += circuit->desequilibre;
    }
    logn4("Solution::update: cost=" +
           U::to_s(this->desequilibre) + "--" +
           U::to_s(this->length));
    logn4("Solution::update END");
}

// Construction d'une chaine correspondant au format standard de la solution
//
// Exemple de format de sortie
//
//     # Fichier velib_0_0_279.sol
//     # generee le ... par ...
//
//     nom velib_0
//     desequilibre 0
//     distance 270
//
//     # id, charge_init, desequ, longueur
//     circuit r1 0 0 58
//     s1 0
//     end
//
//     # id, charge_init, desequ, longueur
//     circuit r2 6 0 212
//     s4 2
//     s2 4
//     s3 -3
//     s5 0
//     s6 -1
//     s7 0
//     s8 -1
//     end
//
string Solution::to_s() {
    /// cout << "# solution pour " << inst->name << endl;
    ostringstream buf;
    buf << "# générée le : TODO " << endl;
    buf << endl;
    buf << "nom "          << inst->name         << endl;
    buf << "desequilibre " << this->desequilibre << endl;
    buf << "distance "     << this->length       << endl;
    buf << endl;
    for (unsigned i=0; i<circuits->size(); i++) {
        Circuit* circuit = circuits->at(i);
        buf << circuit->to_s() << endl;
    }
    return buf.str();
}
// Fornat libre et verbeux
string Solution::to_s_long() {
    // U::die("Solution::to_s_long : non implémentée");
    ostringstream buf;
    buf << "# générée le : TODO " << endl;
    buf << endl;
    buf << "nom "          << inst->name         << endl;
    buf << "desequilibre " << this->desequilibre << endl;
    buf << "distance "     << this->length       << endl;
    buf << endl;
    for (unsigned i=0; i<circuits->size(); i++) {
        Circuit* circuit = circuits->at(i);
        buf << circuit->to_s_long() << endl;
    }
    return buf.str();
}

// Construit un nom de fichier de la forme
//    gotic_9_t20_j100_c03_s40_nn_mmmm.sol
// avec
//    nn : nombre d ejobs non servi
//    mmmm : longueur de la solution
//
string Solution::get_tmp_filename() {
    ostringstream buf;
    buf << inst->name << "-" << this->desequilibre << "-" << this->length
        << ".sol";

    return buf.str();
}

// Exploite la solution en fonction des options de l'appli
void
/// Solution::main_print_solution(Solution* sol, Args* args) {
Solution::main_print_solution(Solution* sol, Options* args) {
    if (log1()) {
        // if (log3()) {
        //     cout << "Affichage détaillé de la solution\n";
        //     cout << sol->to_s_long();
        // }
        // cout << "Affichage de la solution\n";
        // cout << sol->to_s();
        cout << "Affichage détaillé de la solution\n";
        cout << sol->to_s_long();
    } else if (log1() ) {
        /// todo ?
    }
    logn3("Enregistemenet éventuel de la solution\n");
    if (args->outfilename != "") {
        if (args->outfilename == "_AUTO_") {
            // Deux possibilités
            // - ou bien un fichier d'entrée est spécifié et on l'utilise
            // - ou bien l'instance est générée sans être enregistrée
            //   Dans ce cas on basera le fichier de sortie sur le nom de
            //   l'instance avec le nombre de jobs non déservi et la distance
            //   (e.g gotic_9_t20_j100_c03_s40-0-2357.sol)
            //
            if (args->filename != "") {
                // On supprimer le répertoire prefix du fichier d'entree pour
                // que l'enregistreement se fasse dans le repertoire courant.
                // puis on supprime le suffixe ".dat"
                ostringstream buf;
                buf << U::file_basename(args->filename, ".dat")
                    << "-" << sol->desequilibre
                    << "-" << sol->length
                    << ".sol";
                // ATTENTION, ICI ON MODIFIE LA CLASSE args
                args->outfilename = buf.str();
            } else {
                // args->outfilename = sol->inst->name + ".sol";
                args->outfilename = sol->get_tmp_filename();
            }
        }
        U::write_file(args->outfilename, sol->to_s());
    }

}

//./
