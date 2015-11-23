#ifndef ARC_H
#define ARC_H

#include "common.hpp"
#include "site.hpp"


// La classe Arc représente un couple **ORIENTÉ** de site.
// Les boucles sont autorisées.

class Arc {

// les "friend" en premier
friend ostream& operator<<(ostream& os, const Arc  &arc) ;

public:
    std::string name;
    Site* src;
    Site* dst;

    int dist;

    Arc();
    Arc(Site* src, Site* dst);
    virtual ~Arc();

    // Retourne true ssi le passage d'un site à un autre est possible
    // compte tenu : (A COMPLETER)
    // Pour l'instant pas de filtrage : on autorise tous les arcs
    bool is_feasible() {return true;};

    // vérifie si site est une des extrémités de l'arete
    bool is_adj(Site* site);

    // Retourne l'extrémité de l'arete opposée à site
    Site* get_opposite(Site* site);

    std::string to_s();
    std::string to_s_long();
};
#endif

