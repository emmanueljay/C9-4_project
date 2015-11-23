#include "arc.hpp"
#include "instance.hpp"

// les "friend" en premier
ostream& operator<<(ostream& os, const Arc  &arc) {
    return os << arc.name;
}

Arc::Arc(Site* src, Site* dst) {
    this->src = src;
    this->dst = dst;
    this->name = "[" + src->name + "->" + dst->name + "]";

    double x1 = src->x;
    double y1 = src->y;
    double x2 = dst->x;
    double y2 = dst->y;
    dist = (int)round(sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)));

}
// Destructeur
Arc::~Arc() {
}

bool
Arc::is_adj(Site* site) {
   return  (this->src == site || this->dst == site );
}
Site* Arc::get_opposite(Site* site) {
    Site* opposite=NULL;
    if (this->src == site) {
        opposite = this->dst;
    } else if (this->dst == site) {
        opposite = this->src;
    } else {
        // Il faudra lever une vraie exception. Pour l'instant on hurle !!
        cerr << "ERREUR : get_opposite  de " << this->to_s()
             << " pour " << site->to_s()  << " : \nsite non adjacent !!\n";
    };
    return opposite;
}

// Affichage plus détaillé que le simple name
string Arc::to_s() {
    return name;
}
// Affichage plus détaillé que le simple name
string Arc::to_s_long() {
    ostringstream stm;
    stm << name << ": "
        << " dist=" << dist ;
    if (! this->is_feasible()) {
        stm << " ARC INFAISABLE !";
    }
    return stm.str();
}
//./
