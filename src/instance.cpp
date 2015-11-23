#include "instance.hpp"

// Voir fichier instance.hpp pour les exemple de fichier d'instance et de solution
//

void Instance::init() {
    this->args = NULL;
    this->name = "";
    this->filename = "";

    this->nb_remorques = 0;
    this->nb_stations = 0;
    this->nb_sites = 0;

    this->remorques = new vector<Remorque*>();
    this->stations = new vector<Station*>();
    this->sites = new vector<Site*>();

    // this->best_sol = new Solution(this);
}

Instance::Instance() {
    this->init();
}

/// Instance::Instance(string filename, Args* args) {
Instance::Instance(string filename, Options* args) {
    this->init();
    this->filename = filename;
    this->args = args;
    this->read_velib_instance();
    this->build_arcs();
}

Instance::~Instance() {
    // Les destructions à faire
    // - delete des sites et vecteur de sites (stations et remorques)
    // - delete des objets de arcs et de arcs
    // - vider la map arcs_map et delete de arcs_map
    // - vider le tableau dist et delete dist
    // Mais pour l'instant on ne crée qu'une seule Instance par exécution
    // => destruction automatique en fin de programme

    // for (vector<Site>::iterator it = sites->begin(); it != sites->end() ;it++) {
    //     delete *it
    // }
    vector<Site*>::iterator it = sites->begin();
    while (it != sites->end()) {
        delete *it;
        it++;
    }
    delete sites;
    delete remorques;
    delete stations;
    for (vector<Arc*>::iterator it = arcs->begin(); it != arcs->end(); it++) {
        delete *it;
    }
    delete arcs;
    // delete arcs_map;
    delete arcs_grid;
}

void Instance::read_velib_instance() {
    Log::level -= 3;

    // Initialisation de la variable d'état "state" (INUTILE ICI !)
    //
    // Cette variable d'état de type énuméré (ou assimilé) permet d'adapter
    // le traitement du fichier en fonction de la zone du fichier qui a déjà été lu.
    //
    // Cependant, dans le format d'instance de ce problème (ie. gotic), chaque
    // ligne est indépendante : on n'a pas besoin connaitre l'historique de
    // l'analyse pour savoir comment interpréter une ligne.  On n'a donc pas
    // besoin d'une variable d'état pour représenter le passé.
    //
    // Exemples de différents états possibles :
    // "HEADER_STATE" :
    //      état initial, et en cours lecture en-tete
    // "STATION_STATE" :
    //      en cours lecture de la liste de noeuds (stations)
    // "DONE_STATE" :
    //      quand l'ensemble des stations est lu
    //
    // string state = "HEADER_STATE";

    int line_idx = 0;


    vector<string> lines = U::read_lines(this->filename);
    // vector<string>::iterator  it_line = lines.begin();
    auto  it_line = lines.begin(); // C++11 seulement
    while (it_line != lines.end()) {
        string line = *it_line;
        it_line ++;
        line_idx ++;
        if (log5()) {
            cout << "#### line:" << line_idx << " brute (" << line.size() << " chars) => \"" << line << "\"\n";
        }
        line = U::trim_string(line);
        if (log4()) {
            cout << "#### line:" << line_idx << " (" << line.size() << " chars) => \"" << line << "\"\n";
        }

        if (line.empty()) {
            logn4("#### Ligne blanche : on saute.");
            continue;
        }
        if (line.find("//") == 0  || line.find("#") == 0) {
            logn4("#### Ligne commentaire : on saute.");
            continue;
        }
        if (line == "END") {
            logn4("#### Fin normale de fichier (mot clé \"END\")");
            break;
        }
        // On doit traité une ligne utile

        // La ligne doit être de la forme "key xxxx xxx xxx"
        string key = "";
        string val = "";
        if (! U::extract_key_val(line, key, val)) {
            cerr << "\nFormat de ligne incorrect pour la ligne \"" << line << "\"" << endl;
            exit(1);
        }
        if (log4()) {
            cout << "     Détection key=" << key << " val=\"" << val << "\"\n";
        }
        //
        // On connait les chaines key et val
        // - key peut contenir "version",  "remorque", ...
        // - val peut contenir "1.0", "r1  25  25  6", "s1  10  50  11  8  8"...
        //

        if (key == "velib") {
            stringstream buf(val);
            buf >> name;
            buf >> nb_remorques;
            buf >> nb_stations;
            logn4("     Extraction de name => name=" + name +
                  " nb_remorques=" + ::to_s(nb_remorques) +
                  " nb_stations=" + ::to_s(nb_stations) +   "\n");
            continue;

        } else if (key == "version") {
            stringstream buf(val);
            float version; // ne sera pas utilisé
            buf >> version;
            logn4("     Extraction de version=" + ::to_s(version) + "\n");
            continue;

        } else if (key == "remorque") {
            stringstream buf(val);
            this->add_remorque(val);
            continue;

        } else if (key == "station") {
            this->add_station(val);
            continue;

        // } else if (key == "end") {
        //     // On ignore le reste (éventuel) du fichier
        //     break;

        } else {
            cout << "clé inconnu pour la ligne \"" << line << "\n" << endl;
            exit(1);
        }
    }  // end while

    //
    // Test de cohérence de l'instance
    //
    if ((unsigned)nb_remorques != remorques->size()) {
        cerr << "ATTENTION nb_remorques=" << nb_remorques
             <<  " INCOHERENT AVEC LE NOMBRE DE "
             << "REMORQUES LUES : " << remorques->size() << endl;
    }
    if ((unsigned)nb_stations != stations->size()) {
        cerr << "ATTENTION nb_stations=" << nb_stations <<  " INCOHERENT AVEC LE NOMBRE DE "
             << "STATIONS LUES : " << stations->size() << endl;
    }
    this->nb_sites = this->nb_remorques + this->nb_stations;

    logn2(::to_s(line_idx) + " lignes ont ete lues.");
    Log::level += 3;
}

void Instance::add_remorque(string partial_line) {
    stringstream stm(partial_line);
    string name ;           stm >> name;
    int x ;                 stm >> x ;
    int y ;                 stm >> y ;
    int capa ;              stm >> capa ;

    Remorque* remorque = new Remorque(this, name, x, y, capa);
    logn4("Remorque créé : " + remorque->to_s_long());
    remorques->push_back(remorque);
    sites->push_back(remorque);
}
void Instance::add_station(string partial_line) {
    stringstream stm(partial_line);
    string name ;           stm >> name;
    int x ;                 stm >> x ;
    int y ;                 stm >> y ;
    int capa ;              stm >> capa ;
    int ideal ;             stm >> ideal ;
    int nbvp ;              stm >> nbvp ;

    Station* station = new Station(this, name, x, y, capa, ideal, nbvp);
    logn4("Station créée : " + station->to_s_long());
    stations->push_back(station);
    sites->push_back(station);
}
// Construction des collections (listes, ...) d'arcs de l'instance.
// On suppose que tous les sites (Remorques ou Station) sont créés.
//
// Rappel :
//   Le champs sid est unique pour un Site donc une Remorques et une Station ne
//   peuvent jamais avoir le même id.
//
// - TODO arcs_grid est un vector représentant une matrice dont les coordonnées
//   sont les sid (interne) des sites et dont la valeur est l'arc entre ces deux
//   sites.
//   Les valeurs inférieures ou égales à la diagonale principale sont non
//   définies !
//   Sera utilisé pour accéder à un arc correspondant à partir de deux sites connus
//
// - arcs est un vector des arcs créés
//
// Principe :
// On crée **tous** les arcs (même si non valides)
// Les arcs inverses sont également créés car les deux boucles imbriquées
// vont de 0 à nb_site-1.
//
void Instance::build_arcs() {
    // arcs_map = new map<int, Arc*>();
    arcs_grid = new vector<Arc*>(nb_sites*nb_sites);
    arcs = new vector<Arc*>(); // liste des arcs valides
    dists_grid = new vector<int>(nb_sites*nb_sites);
    for (int i1=0; i1 < nb_sites; i1++) {
        Site* site1 = sites->at(i1);
        for (int i2=0; i2 < nb_sites; i2++) {
            Site* site2 = sites->at(i2);
            Arc* arc = new Arc(site1, site2);
            logn7("Arc créé : " + arc->to_s_long());
            arcs_grid->at(nb_sites*site1->sid + site2->sid) = arc;
            // (*arcs_map)[i1*nb_sites + i2] = arc;
            dists_grid->at(nb_sites*site1->sid + site2->sid) = arc->dist;
            if (arc->is_feasible()) {
                arcs->push_back(arc);
            }
        }
    }
}
// int Instance::getDist(Site* s1, Site* s2) {
//     return (int)round(sqrt((s1->x-s2->x)*(s1->x-s2->x) + (s1->y-s2->y)*(s1->y-s2->y)));
// }

// Attention, l'arc retourné n'est pas forcément valide !
Arc* Instance::get_arc(int i1, int i2) {
    // if (i1==i2) {
    //     cerr << "ERREUR : get_arc(i1,i2) avec i1=i2=" << i1 << endl;
    //     exit(1);
    // };
    // return (*arcs_map)[i1 * this->nb_sites + i2];
    return (*arcs_grid)[i1 * this->nb_sites + i2];
}

Arc* Instance::get_arc(Site* s1, Site* s2) {
    return get_arc(s1->sid, s2->sid);
}
int Instance::get_dist(Site* s1, Site* s2) {
    return (*dists_grid)[s1->sid * this->nb_sites + s2->sid];;
}

// Construit les arcs associés aux stations et à la remorque en paramètre,
// et remplir le vecteur d'arcs fourni.
//
// Si remorque est non NULL, les arcs correspondent aux circuit fermé de la
// Remorque desservant les stations.
// Si remorque est NULL, les arcs correspondent au chemin (ouvert reliant les
// sites (qui sont des stations) dans l'ordre.
//
// La gestion du vector arcs (création et suppression) est entièrement à la
// charge de l'appelant.
void Instance::fill_arcs(vector<Arc*>* arcs,
               list<Station*>* stations,
               Remorque* remorque) {
    logn6("Instance::fill_arcs BEGIN");
    arcs->clear();
    if (stations->empty()) {
        return;
    }
    if (remorque != NULL) {
        arcs->push_back(this->get_arc(remorque, stations->front()));
    }
    // logn3("Instance::fill_arcs stations: avant front " );
    Site* src = stations->front();
    // for (unsigned s = 1; s < stations->size(); s++) {
    //     Site* dst = stations->at(s);
    //     Arc* arc = this->get_arc(src, dst);
    //     arcs->push_back(arc);
    //     src = dst;
    // }
    Site* dst = NULL;
    for (auto it = stations->begin(); it != stations->end(); it++) {
        dst = *it;
        // logn3("Instance::fill_arcs stations: avant front " + U::to_s(*dst));
        Arc* arc = this->get_arc(src, dst);
        arcs->push_back(arc);
        src = dst;
    }
    if (remorque != NULL) {
        // arcs->push_back(this->get_arc(stations->back(), remorque));
        arcs->push_back(this->get_arc(dst, remorque));
    }
    logn7("Instance::fill_arcs stations: " + this->to_s(stations) );
    logn7("Instance::fill_arcs arcs: " + this->to_s(arcs) );
    logn6("Instance::fill_arcs END");
}

// Calcule la distance de parcours correspondant aux stations et à la remorque
// en paramètre
//
// Si remorque est non NULL, la distance correspond aux circuit fermé de la
// Remorque desservant les stations.
// Si remorque est NULL, la distance correspondent au chemin ouvert reliant les
// sites (qui sont des stations) dans l'ordre.
//
int Instance::get_dist(list<Station*>* stations, Remorque* remorque) {
    logn6("Instance::get_dist stations BEGIN");
    if (stations->empty()) {
        return 0;
    }
    int dist = 0;
    Site* src = stations->front();
    Site* dst = NULL;
    // On commence à partir de la seconde station grâce au ++ initial
    for (auto it = ++stations->begin(); it != stations->end(); it++) {
        dst = *it;
        dist += this->get_dist(src, dst);
        src = dst;
    }
    if (remorque != NULL) {
        dist += this->get_dist(remorque, stations->front());
        dist += this->get_dist(stations->back(), remorque);
    }
    logn6("Instance::get_dist END avec dist=" + U::to_s(dist));
    return dist;
}

// Affiche l'instance courante dans un format réutilisable en entrée
//
void Instance::print_instance(ostream& os) {
    os << to_s();
}
// Enregistre l'instance courante dans un fichier
// Attention : si le fichier existe déjà : il sera ecrasé !
//
void Instance::print_instance(string filename) {
    ofstream fid(filename.c_str(), ios::out);
    if (! fid) {
       string msg = "impossible de créer \"" + filename + "\"";
       cerr << msg;
       exit(1);
    }
    fid << to_s();
}
string Instance::to_s() {

    stringstream buf;
    buf << "# Instance Velib regeneree" << endl;
    buf << "velib " << name
        << " " << remorques->size()
        << " " << stations->size() << endl;
    buf << "version 1.0 " << endl;
    buf << endl;

    buf << "#remorque  id    x   y    K" << endl;
    for (unsigned i = 0; i < remorques->size(); i++) {
        buf << remorques->at(i)->to_s() << endl;
    }
    buf << endl;

    buf << "#station   id    x   y    capa ideal nbvp" << endl;
    for (unsigned i = 0; i < stations->size(); i++) {
        buf << stations->at(i)->to_s() << endl;
    }

    // buf << "END" << endl;

    return buf.str();
}

void Instance::test_get_arc(Instance* b) {
    cout << "Test acces arc (1,2) : to_s, src, dst et dist\n";
    // Edge* edge = b->get_edge(1,2);
    cout << "pp\n";
    cout << "to_s()=     " << b->get_arc(1,2)->to_s() << endl;
    cout << "src->to_s()=" << b->get_arc(1,2)->src->to_s() << endl;
    cout << "src->to_s()=" << b->get_arc(1,2)->dst->to_s() << endl;
    cout << "dist=       " << b->get_arc(1,2)->dist << endl;
}

Arc* Instance::find_arc(string s_name, string d_name) {
    Arc* arc = NULL;
    for (unsigned a = 0; a < this->arcs->size(); a++) {
        Arc* candidate = this->arcs->at(a);
        if (candidate->src->name == s_name && candidate->dst->name == d_name) {
            arc = candidate;
            break;
        }
    }
    return arc;
}

Instance* Instance::new_velib_mini() {
    // U::die("new_velib_mini: non implémentée !");

    // On crée l'instance mini du sujet velib (2 remorques pour 8 stations).
    // Les stations ou remorques peuvent être ajoutées dans un ordre arbitraire.
    Instance* inst = new Instance();
    inst->name = "mini_gen";
    inst->add_remorque("r2 75     75 4");
    inst->add_station(" s2    20 90   9 7 3  ");

    inst->add_remorque("r1   25 25 5");
    inst->add_station("s1   10 50   11 8 8");

    inst->add_station("s3   25  60      10     7   10");
    inst->add_station("s4   45  80       6     4    5");
    inst->add_station("s5   60  60       7     5    7");
    inst->add_station("s6   60  20       6     2    6");
    inst->add_station("s7   80  50       7     5    7");
    inst->add_station("s8   95  85       7     7    2");

    // Je choisis de trier les tableau de Remorque ou de Station sur le champ name.
    // La solution suivante utilise une fonction anonyme pour le tri. Elle
    // nécessite cependant une version récente d'un compilateur c++.
    //   sort(inst->stations->begin(),
    //        inst->stations->end(),
    //        [](const Station*  a, const Station*  b) ->bool{
    //               return a->name < b->name;
    //        }
    //   );
    sort(inst->stations->begin(), inst->stations->end(), Site::compareByName);
    sort(inst->remorques->begin(), inst->remorques->end(), Site::compareByName);

    return inst;
}

//./

