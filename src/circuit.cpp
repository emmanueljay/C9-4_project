#include "circuit.hpp"

Circuit::Circuit(Instance* inst, Remorque* remorque) {
    // logn1("Circuit::Circuit: START " + remorque->name);
    this->inst = inst;
    this->remorque = remorque;
    this->stations = new list<Station*>();
    this->charge_init = 0;
    this->desequilibre = 0;
    this->length = 0;

    this->depots = new map<Station*,int>();
    this->charges = new map<Station*,int>();
    // this->depots = new vector<int>();
    // this->charges = new vector<int>();
}
Circuit::Circuit(Instance* inst, Remorque* remorque, list<Station*>* stations)
        : Circuit(inst, remorque) {
    // logn1("Circuit::Circuit: START " + remorque->name +
    //                                  U::to_s(count(stations)) +
    //                                  "stations.");

    // this(inst, remorque); // INCORRECT
    this->stations->assign(stations->begin(), stations->end());
    /// this->depots->resize(this->stations->size(), 0);  // ssi vector
    /// this->charges->resize(this->stations->size(), 0); // ssi vector
}

// Attention : ceci n'est pas le constructeur par copie car il prend un
// **pointeur** en parametre
Circuit::Circuit(const Circuit* other) {
    this->inst = other->inst;
    this->remorque = other->remorque;
    this->stations = new list<Station*>(*other->stations);  // ATTENTION aux *
    this->charge_init = other->charge_init;
    this->desequilibre = other->desequilibre;
    this->length = other->length;

    this->depots = new map<Station*,int>(*other->depots);  // ATTENTION aux *
    this->charges = new map<Station*,int>(*other->charges);  // ATTENTION aux *
    // this->depots = new vector<int>(*other->depots);  // ATTENTION aux *
    // this->charges = new vector<int>(*other->charges);  // ATTENTION aux *
}

Circuit::~Circuit() {
    delete this->stations;
    delete this->depots;
    delete this->charges;
}

void Circuit::copy(Circuit* other) {
    this->remorque = other->remorque;
    this->stations->assign(other->stations->begin(), other->stations->end());
    this->charge_init = other->charge_init;
    this->desequilibre = other->desequilibre;
    this->length = other->length;

    // NON TESTÉ
    *(this->depots) = *(other->depots);
    *(this->charges) = *(other->charges);
}

void Circuit::partial_clear() {
    this->charge_init = 0;
    this->desequilibre = 0;
    this->length = 0;
    this->depots->clear();
    // this->depots->resize(this->stations->size(), 0); // ssi vector
    this->charges->clear();
    // this->charges->resize(this->stations->size(), 0); // ssi vector
}

void Circuit::clear() {
    this->partial_clear();
    this->stations->clear();
}

// Pour l'instant, ne fait rien d'autre que l'équilibrage
void Circuit::update() {
    logn5("Circuit::update BEGIN");
    // U::die("Circuit::update : non implémentée");
    this->partial_clear();
    logn6("Circuit::update: equilibage pour " + U::to_s(*remorque));
    this->equilibrate();

    // Mise à jour distance parcourue totale et déséquilibre global
    logn6("Circuit::update: mise à jour des distances du circuit " + U::to_s(*remorque));
    this->length = inst->get_dist(this->stations, this->remorque);
    logn5("Circuit::update END");
}

// Méthode d'équilibrage d'un circuit
//
void Circuit::equilibrate() {
    logn6("Circuit::equilibrate BEGIN");

    // U::die("Circuit::equilibrate : non implémentée");
    // TODO :
    // version de base minimaliste non optimale mais correcte :
    // - on part avec une remorque vide,
    // - rien n'est déposé en chaque station,
    // - on déduire les déséquibres par station !
    int deposed = 0;
    int sum_deposed = 0;
    for (auto it = this->stations->begin(); it != this->stations->end(); ++it) {
        Station* station = *it;
        logn7(station->to_s_long());
        // la remorque ne dépose rien à cette station
        logn7("Circuit::equilibrate: avant maj depots");

        // Deux solutions pour ajouter un élément dans un pointeur de map, mais
        // aucune n'est élégante !
        //  this->depots->insert(std::pair<Station*,int>(station,0));

        // Si on doit récupèrer trop de vélo        
        if ( station->deficit() < this->charge_init - sum_deposed - remorque->capa)
            deposed = this->charge_init - sum_deposed - remorque->capa;
        // Si on doit poser trop de vélo
        else if (this->charge_init - sum_deposed < station->deficit()) 
            deposed = this->charge_init - sum_deposed;
        // Sinon
        else 
            deposed = station->deficit();
        
        (*this->depots)[station] = deposed;
        sum_deposed += deposed;

        // le nouveau contenu de la remorque reste donc inchangé
        logn7("Circuit::equilibrate: avant maj charges");
        // this->charges->insert(std::pair<Station*,int>(station,this->charge_init));
        (*this->charges)[station] = this->charge_init - sum_deposed;

        // incrémentation du desequilibre du circuit
        logn7("Circuit::equilibrate: avant maj desequilibre");
        this->desequilibre += abs(station->deficit()-deposed);
    }
    // Calcul savant de la charge initiale de la remorque pour garantir les
    // dépots et retraits qui viennent d'être calculé
    this->charge_init = 0; // même si c'est déjà fait par ailleurs !
    logn6("Circuit::equilibrate END");
}

// Insertion d'une station dans un circuit.
// si pos est absent : on ajoute à la fin de la liste des stations
//
// l'appel à ipdate est à la charge de l'appelant
//
void Circuit::insert(Station* station, int pos) {
    logn5("Circuit::insert BEGIN " + station->name + " pos=" + U::to_s(pos));
    if (pos == -1) {
        this->stations->push_back(station);
    } else {
        // on avance l'itérateur jusqu'à la position pos
        auto it = this->stations->begin();
        for (int i = 0; i < pos; ++i) {
            it++;
        }
        this->stations->insert(it, station);
    }
    logn5("Circuit::insert END");
}

// Cette brique d'insertion est opérationnelle (mais suppose que la brique
// d'équilibrage est faite), mais pas nécessairement efficace !
//
void Circuit::insert_best(Station* station) {
    Log::level += 0; // on peut modifier le level juste pour cette méthode...
    logn5("Circuit::insert_best BEGIN " + this->remorque->name +
          " insertion de " + station->name);
    // U::die("Circuit::insert_best : non implémentée");
    int best_cost = 999999999;
    list<Station*>::iterator best_it;
    int best_pos = 0;
    int pos = 0;

    if (this->stations->size() == 0) {
        // circuit vide : il suffit d'insérer la nouvelle station à la fin
        logn6("Circuit::insert_best station VIDE => push_back simple !");
        this->stations->push_back(station);
    } else {
        for (auto it = this->stations->begin();
                  it != this->stations->end(); ++it) {
            Station* s = *it; // La station devant laquelle on va insérer "station"
            logn7("Circuit::insert_best de " + station->name +
                    " avant " +  s->name);
            auto it2 = this->stations->insert(it, station);
            // On doit mettre à jour ce circuit avant d'en extraire le coût !
            this->update();
            int cost = this->get_cost();
            logn7("  Circuit::insert_best : "
                  " this_pos=" + U::to_s(pos) +
                  ", this_cost=" + U::to_s(cost) +
                  " (best_cost=" + U::to_s(best_cost) + ")");
            if (cost < best_cost) {
                best_cost = cost;
                best_pos = pos;
                best_it = it;
                logn6("Circuit::insert_best : MEILLEURE POSITION POUR L'INSTANT "
                      " avant name=" + s->name +
                      " : best_pos=" + U::to_s(pos) +
                      " => best_cost=" + U::to_s(cost));
            } else {
                // logn7("Circuit::insert_best : pas de record"
                //       " avant name=" + s->name +
                //       " this_pos=" + U::to_s(pos) +
                //       ", this_cost=" + U::to_s(cost) +
                //       " (best_cost=" + U::to_s(best_cost)) + ")";
            }
            // On remet le circuit en état avant de passer à station suivante
            this->stations->erase(it2);
            // A LA FIN
            pos++;
        }
        // On procède effectivement à la meilleure insertion
        logn6("Circuit::insert_best : "
              "best_pos=" + U::to_s(best_pos) +
              " avant name=" + (*best_it)->name +
              " => get_cost=" + U::to_s(best_cost));
        this->stations->insert(best_it, station);
    }
    this->update();
    logn6("Circuit::insert_best circuit APRES insertion\n" + this->to_s_long());
    logn5("Circuit::insert_best END");
    Log::level -= 0; // ...on doit restaurer la modification du level
}

string Circuit::to_s() {
    stringstream buf;
    buf << "# Circuit associé à la remorque " <<  remorque->name <<
           " de capa " << remorque->capa << endl;
    buf << "#       id, charge_init, desequ, longueur\n";
    buf << "circuit " << remorque->name
        << "        " << this->charge_init
        << "        " << this->desequilibre
        << "       "  << this->length
        << endl;
    for (auto it = this->stations->begin(); it != this->stations->end(); it++) {
        Station* s = *it;
        // Différentes possibilités pour accéder à l'élément i
        // buf << "  " << s->name << " " << (*this->depots)[s]  << endl;
        buf << "  " << s->name << " " << this->depots->at(s) << endl;
    }
    buf << "end" << endl;
    return buf.str();
}
// Affichage détaillé d'une solution (format non standard mais plus détaillé) !
string Circuit::to_s_long() {
    stringstream buf;
    buf << "# Circuit détaillé associé à la remorque " <<  remorque->name <<
           " de capa "          << remorque->capa << endl;
    buf << "#   charge_init="   << this->charge_init <<endl;
    buf << "#   desequilibre="  << this->desequilibre <<endl;
    buf << "#   distance="      << this->length <<endl;

    Site* src = this->remorque;
    for (auto it = this->stations->begin(); it != this->stations->end(); ++it) {
        Station* dst = *it;
        Arc* arc = inst->get_arc(src, dst);
        int depot = this->depots->at((Station*)arc->dst);
        int charge = this->charges->at((Station*)arc->dst);
        buf << "   " <<  arc->to_s_long()
            << " depot de "  << depot
            << " => charge = " << charge << endl;
        src = dst;
    }
    if (stations->size() != 0) {
        buf << "   " <<  inst->get_arc(stations->back(), remorque)->to_s_long();
    }
    return buf.str();
}
//./

