#ifndef OPTION_PARSER_H
#define OPTION_PARSER_H

// Pour en savoir plus sur cette classe, voir la distrib/doc/demo en :
//
//     http://www.ensta-paristech.fr/~diam/c++/online/option_parser/
//
// hist (complément)
// - 18/11/2015 (diam) : correction mineure de error_handler et divers


#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

// class OptionParser;
// class Option;

class OptionParser {
    // Type de la spécification : une spéc. peut décrire une option (e.g type DOC)
    // ou du texte séparateur utilisé dans l'aide.
    public:
    enum SpecType { UNKNOWN=0, BOOL=1, INTEGER=2, DOUBLE=3, STRING=4, DOC=5,
                    ABBREV=6, SWITCH=7 };

    // uid (unique id) est utilisé pour l'id des pseudo-options (e.g doc)
    int uid;

    // nom de base de l'exécutable
    string exename;

    string abstract;

    // Que faire en cas d'option non déclarée sur ligne de commande ?
    string error_handler;  // possibles:  exit | raise | warn | ignore

    // tableau des arguments sur la ligne de commande
    vector<string> argv;

    // Mémoriser argc permet des prétaitements éventuels
    int argc;

    // liste des paramètres simples (par exemple la listes des fichiers à traiter)
    vector<string> params;

    // pointeur sur la liste des parametres (idem ci-dessus mais c'est un pointeur
    // vers une variable utilisateur)
    vector<string>* user_params;

    class Option  {
        public:

        // error: field has incomplete type 'OptionParser'
        // parser: le parser propriétaire de cette option
        // OptionParser parser; // BUG déclaration
        string id;
        string desc;
        bool required; // à conserver mais si pas exploité

        SpecType type;

        // l'attribut active_value n'est utilisée que pour les options ABBREV et
        // SWITCH qui doivent mémoriser la valeur à affecter.
        // Le type string est suffisant car le traitement se fera en fonction
        // du type de la variable de référence.
        string active_value;

        bool*     bool_ref_variable;
        int*       int_ref_variable;
        double* double_ref_variable;
        string* string_ref_variable;

        Option* ref_option;

        // internal : vrai pour les options dérivées créés par le programme mais
        // n'ayant pas de sens du point de vue de l'utilisateur.
        // La internal=true évite :
        // - l'affichage de la valeur de l'option;
        // - l'affichage de l'aide pour cette option.
        // Par exemple la valeur de --debug n'a pas de sens car c'est la valeur
        // de --verbose-level qui doit être affichée.
        //
        bool internal;

        // Liste des clés disponibles pour une option donnée
        // (e.g. -n, --nbiter, --max_b_iterations...)
        vector<string>* keys;

        // Liste des option de type ABBREV qui sont des abréviation de cette
        // objet
        // e.g. L'option associée à la clé --red (avec sa valeur "red") sera
        // rangée dans dans les listes abbrevs de l'option associée à --color.
        vector<Option*>* abbrevs;

        // Le constructeur
        // Option(OptionParser parser, string id) {xxx} // BUG déclaration
        Option(string id, SpecType type = STRING) {
            // this->parser = parser; // BUG déclaration
            this->id = id;
            this->internal = false;
            this->keys = new vector<string>();
            this->abbrevs = new vector<Option*>();
            this->desc = "MISSING DESCRIPTION";
            this->required = false;
            this->type = type;

            this->active_value   = "ACTIVE_VALUE_UNDEFINED";
            this->ref_option     = NULL;

            this->bool_ref_variable   = NULL;
            this->int_ref_variable    = NULL;
            this->double_ref_variable = NULL;
            this->string_ref_variable = NULL;

        }
        ~Option() {
            delete this->keys;
            delete this->abbrevs;
        }

        Option* set_desc(string desc) {
            this->desc = desc;
            return this;
        }
        Option* set_required(bool req=true) {
            this->required = req;
            return this;
        }
        Option* add_alias(string key) {
            if (this->type == SWITCH) {
                this->add_abbrev(key, !*this->bool_ref_variable);
                /// this->add_abbrev(key, *this->bool_ref_variable);
            } else {
                this->keys->push_back(key);
            }
            return this;
        }
        Option* add_key(string key) {  // idem que add_alias
            return this->add_alias(key);
        }
        // Le paramètre internal est nécessaire pour créer les options de type
        // SWITCH ; car se serait très lourd de récupérer la référence sur l'option de
        // type ABBREV crée par cette méthode add_abbrev de façon à en modifier
        // l'attribut internal après sa création par la classe OptionParser.
        Option* add_abbrev(string key, string value, bool internal=false) {
            Option* abbrev = new Option(key, ABBREV);
            abbrev->active_value = value;
            abbrev->ref_option = this;
            abbrev->internal = internal;
            this->abbrevs->push_back(abbrev);
            return this;
        }
        // quelque facilités (int double) pour éviter de taper une chaine.
        // Mais attention : ne pas créer add_abbrev(string key, double value)
        // sinon elle est appelé à mon insu !?
        //
        Option* add_abbrev(string key, int value, bool internal=false) {
            stringstream buf;
            buf << value;
            return add_abbrev(key, buf.str(), internal);
        }
        Option* add_abbrev(string key, double value, bool internal=false) {
            stringstream buf;
            buf << value;
            return add_abbrev(key, buf.str(), internal);
        }

        // Dans le cas d'un paramètre de type string, on prévoit le
        // cas où le type souhaité est donné par l'attribut type.
        // Si l'attribut type n'est pas STRING, on procède à une conversion.
        // Ceci permet de passer une chaine au moment de l'analyse des arguments
        // aueaue soit le type final.
        //
        // ATTENTION la variante set_value(char const*) est nécessaire sinon bug!!!
        // (i.e. aucune des méthodes set_value(...) ne serait appelée)
        // google : stackoverflow c-style-strings-to-stdstring-conversion-clarification
        Option* set_value(char const* chars) {
            return set_value(string(chars));
        }
        Option* set_value(const string& value) {

            // cout << "xxx set_value " << id << " value=" << value << endl;
            if (this->type==BOOL || this->type==SWITCH) {
                // TODO:  if ( (value == "1") || (value == "true") || (value == "t") ) {...}
                if (value == "1" || value == "true") {
                    *this->bool_ref_variable = true;
                } else if (value == "0" || value == "false") {
                    *this->bool_ref_variable = false;
                } else {
                    cerr << "Erreur : seules les valeurs 0 ou 1 sont supportées pour "
                         << "l'option " << this->id << " (" << value << ")." << endl;
                    exit(1);
                }
            } else if (this->type==INTEGER) {
                if (value.find_first_not_of("0123456789") != string::npos) {
                    cerr << "Erreur : l'option " << this->id
                         << " doit être de type entier décimal." << endl;
                    exit(1);
                }
                stringstream buf(value);
                buf >> *this->int_ref_variable;
            } else if (this->type==DOUBLE) {
                stringstream buf(value);
                buf >> *this->double_ref_variable;
            } else if (this->type==STRING) {
                *this->string_ref_variable = value;
            } else if (this->type==ABBREV) {
                cerr << "ERREUR : méthode set_value() non implémentée pour "
                     << "le type \"" << type << "\"";
                exit(1);
            } else {
                // DOC ou UNKNOWN
                cerr << "ERREUR : méthode set_value() non implémentée pour "
                     << "le type \"" << type << "\"";
                exit(1);
            }
            return this;
        }
        // retourne une représentation chaine du type de l'objet courant.
        static string type_to_s(SpecType type) {
            if (type==UNKNOWN) {
                return "UNKNOWN";
            } else if (type==BOOL) {
                return "BOOL";
            } else if (type==INTEGER) {
                return "INTEGER";
            } else if (type==DOUBLE) {
                return "DOUBLE";
            } else if (type==STRING) {
                return "STRING";
            } else if (type==ABBREV) {
                return "ABBREV";
            } else if (type==SWITCH) {
                return "SWITCH";
            } else if (type==DOC) {
                return "DOC";
            } else {
                return "ERREUR_TYPE_NON_DEFINI";
            }
        }
        string type_to_s() {
            return type_to_s(this->type);
        }
        // retourne une représentation chaine de la valeur courante
        string value_to_s() {
            stringstream buf;
            if (type==BOOL || type==SWITCH) {
                buf << *bool_ref_variable;
            } else if (type==INTEGER) {
                buf << *int_ref_variable;
            } else if (type==DOUBLE) {
                buf << *double_ref_variable;
            } else if (type==STRING) {
                buf << *string_ref_variable;
            } else if (type==ABBREV) {
                buf << *string_ref_variable;
            } else {
                buf << "NO_VALUE";
            }
            return buf.str();
        }
        // Affiche l'état détaillé de l'option
        string inspect() {
            stringstream buf;
            buf << "------------------------------\n";
            if (this->type == DOC) {
                // une option DOC n'a pas de valeur associée
                buf << "id:                 " << id << endl;
                buf << "type:               " << type_to_s() << endl;
                buf << "desc:               " << desc << endl;
                buf << endl;
            } else if (this->type == ABBREV) {
                // La valeur associée à une option ABBREV est celle de sa ref_option
                buf << "id:                 " << id
                    << (this->internal ? " (internal) " : "") << endl;
                buf << "type:               " << type_to_s() << endl;
                buf << "active_value:       " << this->active_value << endl;
                buf << "ref_option:         " << this->ref_option->id << endl;
                buf << "ref_value:          " << this->ref_option->value_to_s();
                buf << endl;
                buf << endl;
            } else {
                // Le type SWITCH est traité comme le type BOOL (ou INT, ...)
                buf << "id:                 " << id
                    << (this->internal ? " (internal) " : "") << endl;
                buf << "alias possibles:    ";
                for (vector<string>::iterator it = keys->begin();
                        it != keys->end(); it++) {
                    buf << *it << ", ";
                }
                buf << endl;
                buf << "type:               " << type_to_s() << endl;
                // required n'est pas encore utilisé mais CONSERVER la ligne suivante
                // buf << "required:        " << required << endl;
                buf << "value:              " << value_to_s() << endl;
                buf << "desc:               " << desc << endl;
                buf << endl;
                if (this->abbrevs->size() >= 1) {
                    for (unsigned i=0; i < this->abbrevs->size(); i++) {
                        buf << this->abbrevs->at(i)->inspect();
                    }
                }
            }
            return buf.str();
        };
        // Affiche l'aide pour cette option
        string get_help() {
            if (this->internal) {
                return "";  // pas d'aide pour une option interne !
            }

            stringstream buf;
            // buf << "-------- Aide d'une option : --------\n";
            // On commence par le cas particulier d'une pseudo-option (e.g. DOC...)
            if (this->type == DOC) {
                buf << this->desc << endl;
                return buf.str();
            } else if (this->type == ABBREV) {
                buf << "    " << this->id << " : abréviation pour ";
                buf << this->ref_option->id << " " << this->active_value;
                buf << endl;
                return buf.str();
            } else {
                if (this->type == SWITCH) {
                    // Affichage d'une aide spécial pour les types SWITCH
                    //
                    // on veut construire "dry-run" à partir de "--dry-run-val"
                    //
                    string base_key =  OptionParser::ltrim_string(this->id, "-");
                    unsigned long idx  = base_key.find_last_of("-"); // xxx-var
                    if (idx != string::npos) {
                        base_key.erase(idx, string::npos); // suppression de "-var"
                    } else {
                        cerr << "id d'option incorrect : " << this->id << " attendu xxx-val\n";
                        exit(1);
                    }

                    buf << "  " << "--[no-]" << base_key
                        << " : flag associé à  ";
                    buf << this->id
                        << " (def: " << this->value_to_s() << ")" ;
                    /// buf << endl;
                } else {
                    // les types d'option standard avec valeur
                    buf << "  " << this->id << " " << this->type_to_s()
                        << "  (def: " << this->value_to_s() << ")";
                }

                // Affichage des alias éventuels de cette option
                if (keys->size() >= 1) {
                    buf << "    alias: ";
                    for (unsigned i=0; i < this->keys->size(); i++) {
                        buf << "" << this->keys->at(i) << ", ";
                    }
                }
                buf << endl;
                buf << "    " << this->desc << endl;
                // Affichage des abréviations éventuelles de cette option
                if (abbrevs->size() >= 1) {
                    for (unsigned i=0; i < this->abbrevs->size(); i++) {
                        Option* abbrev = this->abbrevs->at(i);
                        if (abbrev->internal) continue;
                        buf << "    " << abbrev->id << " : abréviation pour ";
                        buf << abbrev->ref_option->id << " " << abbrev->active_value;
                        buf << endl;
                    }
                }

                return buf.str();
            }
        };
    };
    // liste ordonnée des options
    vector<Option*>* options;

    // Pour accéder facilement à une option à partir d'une clé
    // Dans un premier temps : accès à l'option à partir de l'id (~ name)
    map<string, Option*>* map_options;

    OptionParser(int argc, char *argv[]) {
        this->argc = argc;
        this->options = new vector<Option*>();
        this->params = vector<string>();
        this->user_params = NULL;
        this->map_options = new map<string, Option*>();
        this->exename = argv[0];
        this->error_handler = "exit";
        this->argv = vector<string>();
        for (int i = 1; i < this->argc; i++) {
            this->argv.push_back(string(argv[i]));
            // cout << "DETECTION DU MOT : " << argv[i] << endl;
        }
        this->uid = 0;

        stringstream buf;
        buf << "Syntaxe :\n";
        buf << "  "<< this->exename << " [options...] [--] [file1 [file2 [...]]]\n";
        buf << "  "<< this->exename << " -h pour en savoir plus\n";
        this->abstract = buf.str();

    }
    virtual
    ~OptionParser() {
        delete this->options;
        delete this->map_options;
    }

    private:
    Option* add_option(string id, SpecType type=STRING) {
        Option* opt = new Option(id, type);
        this->options->push_back(opt);
        (*this->map_options)[id] = opt;
        return opt;
    }
    public:

    Option* add_bool_option(string id, bool& var) {
        Option* opt = add_option(id, BOOL);
        opt->bool_ref_variable = &var;
        return opt;
    }
    Option* add_int_option(string id, int& var) {
        Option* opt = add_option(id, INTEGER);
        opt->int_ref_variable = &var;
        return opt;
    }
    Option* add_double_option(string id, double& var) {
        Option* opt = add_option(id, DOUBLE);
        opt->double_ref_variable = &var;
        return opt;
    }
    Option* add_string_option(string id, string& var) {
        Option* opt = add_option(id, STRING);
        opt->string_ref_variable = &var;
        return opt;
    }
    Option* add_abbrev_option(string id, string ref_option_id, string value) { // XXX A VIRER QUAND SERA FINI
        Option* opt = add_option(id, ABBREV);
        opt->active_value = value;
        if (!this->key_exists(ref_option_id)) {
            cerr << "Erreur : aucune option d'id " << ref_option_id << " n'est définie !";
            exit(1);
        }
        Option* ref_opt = this->get(ref_option_id);
        opt->ref_option = ref_opt;
        return opt;
    }
    Option* add_switch_option(string id, bool& var) {
        // La création d'un switch --xxx entraine la création de trois options
        // - une option interne de type BOOL --xxx-val mémorisant la valeur
        // - l'option publique de type SWITCH (variante de ABBREV) --xxx
        //   associée à la valeur 1
        // - une option interne de type ABBREV --no-xxx associé à la valeur 0
        //
        // 1. création de la l'option BOOL pour mémoriser la valeur et la
        //    variable du switch
        string val_key = id + "-val";
        Option* val_opt = add_option(val_key, SWITCH);  // was BOOL
        val_opt->bool_ref_variable = &var;
        val_opt->internal = false; // par défaut

        // 2. création de l'abéviation principale équivalente au switch
        string on_key = id;
        val_opt->add_abbrev(on_key, 1, true); // true pour internal
        // val_opt->abbrevs->at(0)->internal = true; // serait équivalent mais non testé

        // 3. création de l'abréviation pour le switch complémentaire
        // le ltrim_string est nécessaire car on veut --no-dry-run et
        // non pas --no---dry-run
        string off_key = string("--no-") + OptionParser::ltrim_string(id, "-");
        val_opt->add_abbrev(off_key, 0, true); // true pour internal

        return val_opt;
    }
    // Insère du textet au milieur de l'aide sur les options.
    // On peut insérer un simple saut de ligne en ne passant aucun paramètre.
    Option* add_doc(string desc = "") {
        // l'id d'une doc n'a pas d'importance : on utilise donc un uid automatique
        stringstream buf;
        buf << "doc_" << this->uid++;  // on crée un id du type "doc_1"
        string id = buf.str();
        Option* opt = add_option(id);
        opt->type = DOC;
        opt->desc = desc;
        return opt;
    }
    void set_params_vector(vector<string>& user_params) {
        this->user_params = &user_params;
    }
    // Retourne l'objet Option associé à une clé.
    // Cette clé **doit** exister sinon exit !
    Option* get(string id) {
        if (!this->key_exists(id)) {
            cerr << "Erreur : la clé " << id << " n'existe par !\n";
            exit(1);
        }
        return this->map_options->find(id)->second;
    };
    bool key_exists(string id) {
        map<string, OptionParser::Option*>::const_iterator it = this->map_options->find(id);
        if (it != this->map_options->end()) {
            return true; // la valeur est dans it->second
        } else {
            return false;
        }
    };
    // affiche pour chaque clé sa clé de référence (i.e. l'id de l'option associée)
    string map_options_to_s() {
        stringstream buf;
        buf << "Contenu de map_options\n";
        map<string, OptionParser::Option*>::iterator it = this->map_options->begin();
        while (it != this->map_options->end()) {
            Option* opt = it->second;
            string  key = it->first;
            buf << it->first << "  " << opt->type_to_s()  << "   =>   ";
            if (opt->type == ABBREV) {
                buf << opt->ref_option->id << " active_value=" << opt->active_value;
            } else {
                buf << opt->id;
            }
            buf << endl;
            it++;
        }
        return buf.str();
    };
    void on_error(string handler) {
        // vector<string> handlers = {"exit", "raise", "warn", "ignore"}; c++0x only
        string ar[] = {"exit", "raise", "warn", "ignore"};
        vector<string> handlers(ar, ar+4);
        string handlers_help = "exit, raise, warn, ignore";
        if (std::find(handlers.begin(), handlers.end(), handler) != handlers.end()) {
            this->error_handler = handler;
        } else {
            cerr << "Valeur incorrecte pour on_error : " << handler << endl
                 << "Valeurs autorisées : " << handlers_help << endl;
            exit(1);
        }
    }
    void send_error(string txt) {
        if (this->error_handler == "ignore") {
            // rien à faire;
        } else if (this->error_handler == "warn") {
            cerr << txt;
            cerr << " (config: error_handler=exit)" << endl;
        } else if (this->error_handler == "raise") {
            cerr << txt;
            cerr << " (config: error_handler=exit)" << endl;
            // throw new std::string(txt);
            cerr << "TODO: Valeur \"raise\" de error_handler non implémentée :\n";
            cerr << "=> exit " << endl;
            cerr << this->abstract;
            cerr << endl;
            exit(1);
        } else { // "exit"ou tout autre valeur
            cerr << txt;
            cerr << " (config: error_handler=exit)" << endl;
            cerr << this->abstract;
            cerr << endl;
            exit(1);
        }
    }
    // Complète la structure d'options et effectue quelques vérifications
    // sur la cohérence au sein de l'ensemble des options.
    // e.g. deux clés identiques ne doivent pas déclarées, ...
    // ATTENTION : PEUT EFFECTUER UN EXIT EN CAS DE PROBLÈMES
    void close() {
        // On commence par compléter la map avec l'ensemble des clés déclarées
        // On en profite pour vérifier qu'il n'y a pas de répétition de clés
        for (unsigned i=0; i < this->options->size(); i++) {
            Option* opt = this->options->at(i);
            // on recherche parmi les clée (i.e. alias) de l'option opt
            for (unsigned i=0; i < opt->keys->size(); i++) {
                string key = opt->keys->at(i);
                if (this->map_options->find(key) != this->map_options->end()) {
                    cerr << endl;
                    cerr << "ERREUR : pour option#id=" << opt->id << endl;
                    cerr << "  tentative de redéclaration de la clé " << key << endl;
                    cerr << "  qui est déjà déclarée par l'option#id="
                         << this->map_options->find(key)->second->id << " !\n" ;
                    exit(1);
                }
                this->map_options->insert(make_pair(key, opt));
            }
            // on recherche parmi les abbrevs de l'option opt
            for (unsigned i=0; i < opt->abbrevs->size(); i++) {
                Option* abbrev = opt->abbrevs->at(i);
                // La seule clé d'une abbrev est son id !
                if (this->map_options->find(abbrev->id) != this->map_options->end()) {
                    cerr << endl;
                    cerr << "ERREUR : pour option#id=" << opt->id << endl;
                    cerr << "  tentative de redéclaration de la clé " << abbrev->id << endl;
                    cerr << "  qui est déjà utilisée par l'option#id="
                         << this->map_options->find(abbrev->id)->second->id << " !\n" ;
                    exit(1);
                }
                this->map_options->insert(make_pair(abbrev->id, abbrev));
            }
        }
    }
    virtual
    void parse() {
        // construction de la structure d'options et vérif cohérence de la spéc.
        this->close();

        // xxx_hook() n'est utile que si cette classe est utilisée par héritage !
        this->pre_parse_hook();

        // state : position de l'argument en cours de lecture
        //   NORMAL_STATE  : état classique (recherche et extraction d'une option)
        //   POST_KEYS_STATE : il n'y a plus de clé : on accumules les arguments
        //   résiduels (e.g nom de fichiers, ...)
        //
        string state = "NORMAL_STATE";
        unsigned i = 0; // indice du premier paramètre le la ligne de commende
        while (i < argv.size()) {
            string arg = this->argv[i];
            // cout << "xxx i=" << i << "=> arg=" << arg << endl;
            if (state == "NORMAL_STATE") {
                // Traitement d'une option normale
                if (arg == "--") {
                    // la suite ne contient plus de clé, même si un argument commence
                    // par "-xxx"
                    state = "POST_KEYS_STATE";
                    i++;
                    continue;
                } else if (arg == "-h"  || arg == "--help") {
                    this->print_help();
                    exit(0);
                } else if (arg == "---help") {
                    cout << this->inspect();
                    // exit(0); // ON PEUT VOULOIR AALYSER LES RESTE DE LA LdC
                    i++;
                    continue;
                } else if (arg.find("-") == 0) {
                    // arg est une clé. On recherche l'option associée à cette clé
                    // et on vérifie son existance.
                    map<string, Option*>::iterator it = this->map_options->find(arg) ;
                    if (it == this->map_options->end()) {
                        stringstream buf;
                        buf << "Clé inconnue : " << arg;
                        this->send_error(buf.str());
                        i++;
                        continue;
                    }
                    Option* opt = it->second;
                    // On connait l'option associé à cette clé
                    // Il faut distinguer si cette clé est une clé seule (i.e. n'est
                    // pas associés à une valeur) ou si c'est une option standard
                    // associée à une valeur
                    // die();
                    if (opt->type == ABBREV) {
                        // on extrait et on positionne l'option de référence associée
                        // à cette option switch
                        opt->ref_option->set_value(opt->active_value);
                        i++;
                        continue;
                    } else {
                        // il faut extraire la valeur associée à cette clé
                        i++;
                        if (i >= this->argv.size()) {
                            cerr << "Manque la valeur pour la clé \"" << arg << "\"\n";
                            exit(1);
                        }
                        string str_val = argv[i];
                        opt->set_value(str_val);
                        i++;
                        continue;
                    }

                } else {
                    // ce paramètre n'est pas une clé : on l'ajoute à la liste
                    // des "fichiers"
                    this->params.push_back(arg);
                    if (this->user_params != NULL) {
                        this->user_params->push_back(arg);
                    }
                    i++;
                    continue;
                }
            } else if (state == "POST_KEYS_STATE") {
                this->params.push_back(arg);
                if (this->user_params != NULL) {
                    this->user_params->push_back(arg);
                }
                i++;
                continue;
            } else {
                cerr << "ERREUR dans classe OptionParser : état inconnu state="
                     << state << endl;
                exit(1);
            }
        }
        // xxx_hook() n'est utile que si cette classe est utilisée par héritage !
        this->post_parse_hook();
    }
    // Retourne une nouvelle chaine nettoyée de certains caractères sur les bords
    // Par défaut, ces caractères sont espaces ou tabulation
    static
    string ltrim_string(string str, string trim_chars=" \t\r\n") {
        // unsigned int idx  = str.find_first_not_of(chars);
        unsigned long idx  = str.find_first_not_of(trim_chars);
        if (idx == string::npos) {
            // Il n'y a que des caractères à supprimer
            return "";
        } else if (idx == 0) {
            // Aucun caractère à supprimer
            return str;
        } else {
            return str.erase(0, idx);
        }
    }
    virtual
    string inspect() {
        stringstream buf;
        buf << "DETAIL DES OPTIONS DÉFINIES\n";
        for (vector<Option*>::iterator it = options->begin();
                it != options->end(); it++) {
             buf << (*it)->inspect();
        }
        if (this->params.size() != 0) {
            buf << "Liste des " << this->params.size() << " arguments sans clé associés\n";
            buf << "params.size()="<< this->params.size() << endl;
            for (vector<string>::iterator it = this->params.begin();
                    it != this->params.end(); it++) {
                 buf << "    " << *it << endl;
            }
        } else {
            buf << "Liste des fichiers à traiter vide (pas d'arguments supplémentaires\n";
        }

        return buf.str();
    }
    virtual
    ostream& print_dump(ostream& os = cout) {
        os << this->inspect();
        return os;
    }

    // Affiche la syntaxe de l'appli et la description des options
    virtual
    ostream& print_syntaxe(ostream& os = cout) {
        os << this->abstract << endl;

        // On n'affiche cette ligne que si l'appli ne propose pas elle-même en
        // premier une option de type DOC.
        if (this->options->size() != 0 && this->options->at(0)->type != DOC) {
            os << "Liste des options :\n";
        }

        for (vector<Option*>::iterator it = options->begin();
                it != options->end(); it++) {
             os << (*it)->get_help();
        }
        return os;
    }
    // affiche la valeur courante des options
    // - affiche les valeurs par défaut avant l'appel à la méthode parse()
    // - affiche dans l'ordre de la déclaration
    virtual
    ostream& print_values(ostream& os = cout) {
        os << "\nValeurs courantes des options :\n";
        for (vector<Option*>::iterator it = options->begin();
                it != options->end(); it++) {
             if ((*it)->type == DOC) {
                 // on ignore le type DOC
                 continue;
             }
             if ((*it)->ref_option != NULL) {
                 // on ignore les switch (associées à une autre option)
                 continue;
             }
             os << "    " << (*it)->id << " = " << (*it)->value_to_s() << endl;
        }
        if (this->params.size() == 0) {
            os << "Aucun paramètre supplémentaire :\n";
        } else {
            os << "Valeur des paramètres supplémentaires :\n";
            for (unsigned i = 0; i < this->params.size(); i++) {
                cout << "  Param " << i
                     << " : " << this->params[i] << endl;
            }
        }

        return os;
    }
    // get_help: retourne l'aide de l'application.
    // À REDÉFINIR PAR LA CLASSE FILLE
    virtual
    ostream& print_help(ostream& os = cout) {
        return print_syntaxe();
    }
    // pre_parse_hook: permet un prétraitemenet avant d'extraire les options
    // (extraction du premier paramètre à la git ou cvs, ...)
    // À REDÉFINIR PAR LA CLASSE FILLE
    virtual
    void pre_parse_hook() {
    }
    // post_parse_hook: assure les vérification supplémentaire sur les options extraites
    // (validité des valeurs, cohérence pour les option interdépendantes, ...)
    // À REDÉFINIR PAR LA CLASSE FILLE
    virtual
    void post_parse_hook() {
    }

    static
    OptionParser* test(int argc, char *argv[]) {
        // ATTENTION : CETTE METHODE DE TEST N'EST PAS FORCÉMENT À JOUR
        // VOIR les fichier main.cpp et DemoOptions.hpp
        OptionParser* opts = new OptionParser(argc, argv);
        Option* opt;

        cerr << "méthode OptionParser#test() n'est plus à jour ";
        cerr << "A conserver pour les exemples, mais utiliser plutot";
        cerr << "L'exemple de  DemoOptions.hpp et le fichier main.cpp\n";
        // exit(1);

        opt = opts->add_doc("\nDébut des options spécifiques\n");

        int nbiter=10;
        opt = opts->add_int_option("--nbiter", nbiter)
                  ->set_desc("Nombre maximum d'itération")
                  ->add_alias("-i")
                  ->add_alias("--nb-max-iterations");
        // Ce qui suite est seule utilisation de opt pour le compilateir gcc
        cout << "id de la première option crée : " << opt->id;

        double mydouble = 3.14;
        opt = opts->add_double_option("--mydouble", mydouble)
                  ->set_desc("Un flottant de précision double")
                  ->add_alias("-r");

        string infile = "stdin";
        opt = opts->add_string_option("--infile", infile)
                  ->set_desc("Fichier d'entrée")
                  ->add_alias("-f");

        // TODO : implémenter l'option --color et les switch d'abrévations
        string color = "blue";
        opt = opts->add_string_option("--color", color)
                  ->set_desc("Couleurs de fond possibles : blue, write ou red.")
                  ->add_alias("-c")
                  ->add_abbrev("--blue" , "blue")
                  ->add_abbrev("--white", "white")
                  ->add_abbrev("--red"  , "red");
        // vector<string> color_values = {"blue", "write", "red"};
        // this->get_option("color")->set_values(color_values)


        opt = opts->add_doc("\nDébut des options génériques\n");
        int level = 2;
        opt = opts->add_int_option("--level", level)
                  ->set_desc("Niveau de verbosité")
                  ->add_alias("-L")
                  ->add_alias("--verbose-level")
                  ->add_abbrev("--debug", 5)
                  ->add_abbrev("-d", 5);

        bool dry_run = false;
        opt = opts->add_switch_option("--dry-run", dry_run)
                  ->set_desc("Simule l'exécution sans rien faire de dangereux")
                  ->add_abbrev("-n", "1");


        // personnalisation éventuelle du comportement (si option incorrecte)
        cout << "Appel de opts->on_error(\"warn\")\n";
        opts->on_error("warn");  // possibles : exit (par défaut), raise, warn, ignore


        cout << "Appel de opts->print_help()\n";
        opts->print_help(cout);

        cout << "Appel de opts->parse()\n";
        opts->parse();

        cout << "Appel de opts->map_options_to_s()\n";
        cout << opts->map_options_to_s();

        cout << "Appel de opts->inspect()\n";
        cout << opts->inspect();

        cout << "Appel de opts->print_values()\n";
        opts->print_values(); cout << endl;


        // Si on n'est pas sûr de l'existance de l'option :
        cout << "Recherche existance d'une option de clé \"--mydouble\"";
        if (opts->key_exists("--mydouble")) {
            cout << " => existe : " << opts->get("--mydouble")->value_to_s() << endl;
        } else {
            cout << " => N'existe pas !!!" << endl;
        }

        // Si on est sûr de l'existance de l'option :
        cout << "Accès générique via opts->get(\"--mydouble\")." << endl;
        cout << "On peut créer une variable locale ou un attribut pour permettre";
        cout << "un accès direct à chaque option." << endl;
        double thedouble = *(opts->get("--mydouble")->double_ref_variable);
        cout << "  La valeur de thedouble (--mydouble) est : " << thedouble << endl;

        string the_file;
        if (opts->params.size() == 0) {
            cout << "Erreur il faut au moins un nom de fichier en paramètre\n";
        } else {
            the_file = opts->params[0];
        }

        return opts;
    };
};
#endif

