#ifndef SITE_H
#define SITE_H

#include "common.hpp"

class Site {

// les "friend" en premier
friend ostream& operator<<(ostream& os, const Site  &site) ;


public:
    Instance* inst;
    string name;
    int x;
    int y;
    int sid; // interne et automatique

    // identificateur unique, automatique
    static int last_sid;

    // // Quelques attributs dérivés utiles (différents selon Job ou Tic)
    // // Heure de disponibilité au plus tot
    // int earliest_live_time;
    // // Heure de d'arrivée au plus tard
    // int latest_arrival_time;

    // Site();
    Site(Instance* inst, string name, int x, int y);
    virtual ~Site();

    virtual string to_s();
    virtual string to_s_long();

    static int nb_sites() {
        return last_sid;
    }

    virtual string classname() ;
    static string to_s(vector<Site*>* sites);

    static bool compareByName(Site* s1, Site* s2) {
        return s1->name < s2->name;
    }

};


#endif
