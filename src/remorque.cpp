#include "remorque.hpp"

// initialisation des variables statiques a l'exterieur de toute methode
int Remorque::last_id = 0;

string Remorque::classname() {
    return "Remorque";
}

// // les "friend" en premier (si nécessaire)
// ostream& operator<<(ostream& os, const Remorque  &remorque) {
//     return os << remorque.name;
// }

Remorque::Remorque(Instance* inst, string name, int x, int y, int capa)
        : Site::Site(inst, name, x, y) {

    this->id = Remorque::last_id++;
    this->capa = capa;
}
Remorque::~Remorque() {
}

string Remorque::to_s() {
    // TODO : passer par super
    stringstream buf;
    buf << "remorque " << setw(4) << name
        << setw(5) << x << setw(4) << y
        << setw(5) << capa ;
    return buf.str();
}
string Remorque::to_s_long() {
    stringstream buf;
    buf << Site::to_s_long() << " id=" << id << " capa=" << capa;
    return buf.str();
}


//./
