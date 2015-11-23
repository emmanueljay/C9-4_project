#ifndef INSTANCE_H
#define INSTANCE_H

#include "common.hpp"
#include "site.hpp"
#include "station.hpp"
#include "remorque.hpp"
#include "arc.hpp"
#include "options.hpp"
#include "solver.hpp"

// Classe responsable de la manipulation des l'instance (dont lecture du
// fichier d'instance, ...)
//
// Exemple de fichier à reconnaitre :
//
//      velib velib_0 2 8
//      version 1.0
//
//      #remorque   id  x   y   K
//      remorque    r1  25  25  6
//      remorque    r2  75  75  6
//
//      #station  id     x   y  capa  ideal   nbvp
//      station   s1    10  50    11      8      8
//      station   s2    20  90     9      7      3
//      station   s3    25  60    10      7     10
//      station   s4    45  80     6      4      2
//      station   s5    40  60     7      5      5
//      station   s6    60  40     6      5      6
//      station   s7    80  50     7      5      5
//      station   s8    95  85     7      6      7
//
//
class Instance {

public:
    string    name;         // nom de l'instance

    /// Args*     args;         // les options de l'application
    Options*     args;         // les options de l'application

    int       nb_stations;
    int       nb_remorques;


    vector<Station*>* stations;
    vector<Remorque*>* remorques;
    vector<Site*>* sites;
    int       nb_sites;

    // le vecteur des arcs valides
    vector<Arc*>* arcs;

    // arcs_grid est un vector représentant une matrice dont les coordonnées sont
    // les sid (interne) des sites et dont la valeur est l'arc entre ces deux sites
    // Sera utilisé pour accéder à un arc correspondant à partir de deux sites connus
    vector<Arc*>* arcs_grid;

    // Matrice des distances précalculées.
    // Redondant avec l'attribut dist de la classe arcs_grid car chaque arc
    // possède un attribut dist. Mais la notion d'arc n'est probablement pas
    // utile pour le projet velib. L'attribut dists_grid évitera la création des
    // arcs rien que pour récupérer les distances.
    vector<int>* dists_grid;

    // Construteur par défaut
    Instance();
    // Constructeur acceptant un nom de fichier
    Instance(string, Options* args=NULL);

    // Le destructeur
    virtual ~Instance();

    string filename;


    // retourne le meme pointeur de Edge pour i1 < i2 ou i1 > i2
    Arc* get_arc(int i1, int i2);
    Arc* get_arc(Site* s1, Site* s2);
    Arc* find_arc(string s_name, string d_name);

    int get_dist(Site* s1, Site* s2);

    // // construit et retourne un pointeur sur liste d'arcs (voir .cpp)
    // vector<Arc*>* make_arcs(vector<Station*>* stations, Remorque* remorque = NULL);

    // construit les arcs associés au station et à la remorque en paramètre,
    // et remplir le vecteur d'arcs fourni.
    void fill_arcs(vector<Arc*>* arcs, list<Station*>* stations, Remorque* remorque = NULL);

    int get_dist(list<Station*>* stations, Remorque* remorque = NULL);



    // // Mélange le conteneur de classe list passé en parametre
    // // Principe :
    // // 1. on convertit la liste en vector pour pouvoir utiliser la STL
    // // 2. on trie le vector par la lib standard c++
    // // 3. on reconvertit le vector en lists
    // template<typename SITE>
    // void shuffle_sites(list<SITE*>* sites) {
    //     // 1. on convertit la liste en vector pour pouvoir utiliser la STL
    //     // typedef vector<SITE*> Vect;
    //
    //     vector<SITE*>* vect = new vector<SITE*>(sites->size());
    //     // ::copy sans mettre de back_inserter car vector déjà alloué
    //     ::copy(sites->begin(), sites->end(), vect->begin());
    //
    //     // 2. on trie le vector par la lib standard c++
    //     random_shuffle(vect->begin(), vect->end());
    //
    //     // 3. on reconvertit le vector en lists
    //     sites->assign(vect->begin(), vect->end());
    // }


    // // Fonction de tri... TODO passer de Station à Site par template
    // // tri les remorque en fonction de leur proximité à une station donnée
    // void sort_stations_by_nearest_from(vector<Remorque*>* remorques, Site* ref);
    // void sort_stations_by_nearest_from(list<Remorque*>* remorques, Site* ref);


    // Convertit l'instance au format standard velib
    string to_s();

    // Convertit un vecteur de Site (donc de stations ou de remorques) en une
    // chaine sous la forme "[s1, s7, s2, r1, s4]"
    //
    /// string to_s(vector<Station*>* tab);
    /// string to_s(vector<Remorque*>* tab);
    //
    template<typename SITE>
    string to_s(const vector<SITE*>* sites) {
        string sep = ", ";
        string the_sep = "";
        ostringstream buf;
        buf << "[";
        // typename vector<SITE*>::iterator it;
        typename vector<SITE*>::const_iterator it;
        for (it = sites->begin(); it != sites->end(); it++) {
            buf << the_sep << (*it)->name;
            the_sep = sep;
        }
        // Autre syntaxe possible :
        // for (unsigned i=0; i<sites->size(); i++) {
        //     buf << the_sep << sites->at(i)->name;
        //     the_sep = sep;
        // }
        buf << "]";
        return buf.str();
    }
    // Convertit une **liste** de Site (donc de stations ou de remorques) en une
    // chaine sous la forme "[s1, s7, s2, r1, s4]"
    //
    // TODO:
    //   Fusionner ces deux méthode en une seule en paramétrant le type
    //   du conteneur
    template<typename SITE>
    string to_s(const list<SITE*>* sites) {
        string sep = ", ";
        string the_sep = "";
        ostringstream buf;
        buf << "[";
        // typename list<SITE*>::iterator it;
        typename list<SITE*>::const_iterator it;
        for (it = sites->begin(); it != sites->end(); it++) {
            buf << the_sep << (*it)->name;
            the_sep = sep;
        }
        buf << "]";
        return buf.str();
    }
    string to_s(const vector<int>* tab) {
        string sep = ", ";
        string the_sep = "";
        ostringstream buf;
        buf << "[";
        buf << "size=" << tab->size() << " ";
        vector<int>::const_iterator it;
        // auto it;
        for (it = tab->begin(); it != tab->end(); it++) {
            buf << the_sep << *it;
            the_sep = sep;
        }
        buf << "]";
        return buf.str();
    }
    // enregistre l'instance dans un fichier (pour le gérérateur d'instance)
    void print_instance(string filename);

    // affiche l'instance sur un flux ("cout"par défaut)
    void print_instance(ostream& = cout);

    static void test_get_arc(Instance* b);

    // génère l'instance mini donné en exemple dans le sujet
    static Instance* new_velib_mini();

    void add_remorque(string line);
    void add_station(string line);

private:
    void init();

    void read_velib_instance();

    // Création des matrice d'arcs et des distances
    void build_arcs();

};

#endif
//./
