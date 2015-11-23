#ifndef REMORQUE_H
class Remorque;
#define REMORQUE_H

#include "common.hpp"
#include "site.hpp"
#include "instance.hpp"

class Remorque : public Site {

public:
    int id; // interne et automatique

    // identificateur unique, automatique
    static int last_id;

    int capa;

    Remorque(Instance* inst, string name, int x, int y, int capa);
    virtual ~Remorque();
    virtual string classname();

    virtual string to_s();
    virtual string to_s_long();

};
#endif

