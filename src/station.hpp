#ifndef STATION_H
#define STATION_H

#include "common.hpp"
#include "site.hpp"
#include "instance.hpp"

class Station : public Site {

public:

    int id; // interne et automatique

    // identificateur unique, automatique
    static int last_id;

    int capa; // capacité maxi de la station
    int ideal; // nombre idéla de vélo dans la station
    int nbvp;   // nombre de véo présent

    Station(Instance* inst, string name, int x, int y,
                  int capa, int ideal, int nbvp);
    virtual ~Station();
    virtual string classname();

    virtual int margin();
    virtual int deficit();

    virtual string to_s();
    virtual string to_s_long();

};
#endif
