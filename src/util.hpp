#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

//
// Quelques fonctions utilitaires
// Ce fichier ne doit pas dépendre de l'application
//
class U {

    public:

    // arret bestiale du programme (pour debug)
    static void
    die(string msg="die : FIN VOLONTAIRE DU PROGRAMME") {
        std::cerr << msg << endl;
        exit(1);
    }


    // Enregistre la chaine txt dans le fichier de nom filename
    static void
    write_file(string filename, string txt) {
        ofstream fid(filename.c_str(), ios::out);
        if (! fid) {
            string msg = "impossible de créer \"" + filename + "\"";
            cerr << msg << endl;
            exit(1);
        }
        fid << txt << endl;
    }
    // Retourne le contenu du fichier de nom filename
    static string
    read_file(string filename) {
        string txt = "";
        ifstream fid(filename.c_str());
        if (! fid) {
            string msg = "impossible d'ouvrir le fichier \"" + filename + "\"";
            cerr << msg << endl;
            exit(1);
        }
        std::stringstream ss;
        ss << fid.rdbuf();
        return ss.str();
    }
    // Retourne true ssi le fichier existe
    static bool
    file_exists(string filename) {
        ifstream ifs(filename.c_str());
        // 20/01/2014 : correction bug avec clang++ 4.0 (Malverick)
        // return ifs;
        return (bool)ifs;

    }
    // Retourne le mom de base du fichier, débarassé du répertoire et de
    // tout ce qui suite le suffixe.
    //
    // Exemple :
    //   cout << U::file_basename("../data/instance_1.dat");
    //   =>  "instance_1.dat"
    //   cout << U::file_basename("../data/instance_1.dat", ".dat");
    //   =>  "instance_1"
    //   cout << U::file_basename("../data/instance_1.dat". "nstance_1.dat");
    //   =>  "i"
    //   cout << U::file_basename("../data/instance_1.dat", "instance_1.dat");
    //   =>  "instance_1.dat"  (rien n'est supprimée)
    //   cout << U::file_basename("../data/instance_1.dat", ".");
    //   =>  "instance_1"      (supprime ce qui suit le **dernier** point
    //
    static string
    file_basename(string filename, string suffix = "") {
        string basename = filename;
        unsigned long idx = filename.find_last_of("/");
        if (idx != string::npos) {
            basename = basename.substr( idx + 1 );
        }
        if (suffix != "") {
            // On supprime le suffixe du nom de l'instance
            idx = basename.rfind(suffix);
            // le suffixe n'est supprimé s'il correspond au nom complet
            if (idx != string::npos && idx != 0) {
                basename = basename.substr(0, idx);
            }
        }
        return basename;
    }

    // retourne le contenu du fichier sous forme d'aun vecteur de ligne
    static vector<string>
    read_lines(string filename) {
        ifstream fid(filename.c_str(), ios::in);
        if (! fid) {
            string msg = "impossible d'ouvrir \"" + filename + "\"";
            cerr << msg << endl;
            exit(1);
        }

        // Le vecteur qui contiendra toutes les lignes du fichier
        vector<string> lines;
        string oneLine;
        while (getline(fid, oneLine)) {
            // cerr << "=== (" << oneLine.size() << ") " << oneLine << "\n";
            lines.push_back(oneLine);
        }
        fid.close();
        return lines;
    }
    // Retourne une nouvelle chaine nettoyée de certains caractères sur les bords
    // Par défaut, ces caractères sont espaces ou tabulation
    //
    static string
    ltrim_string(string str, string trim_chars=" \t\r\n") {
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
    // Nettoyage des bords de chaine de caracteres.
    // e.g;
    //      line =  U::trim_string(line)
    //
    static string
    rtrim_string(string str, string trim_chars=" \t\r\n") {
        // unsigned int idx  = str.find_last_not_of(chars);
        unsigned long idx  = str.find_last_not_of(trim_chars);
        if (idx != string::npos) {
            str.erase(idx+1, string::npos);
        }
        return str;
    }
    static string
    trim_string(string str, string ltrim_chars, string rltrim_chars) {
        string str2 = ltrim_string(str, ltrim_chars);
        return rtrim_string(str2, rltrim_chars);
    }
    static string
    trim_string(string str, string  trim_chars=" \t\r\n") {
        return trim_string(str, trim_chars, trim_chars);
    }

    // méthode d'aide à l'analyse de fichier de parametre
    //
    static bool
    extract_key_val(string line, string &key, string &val,
                   string key_letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_"
                                        "abcdefghijklmnopqrstuvwxyz",
                   string ltrimchars=" \t=:",
                   string rtrimchars=" \t\r\n;,") {

            // unsigned int idx  = line.find_first_not_of(key_letters);
            unsigned long idx  = line.find_first_not_of(key_letters);
            if (idx == 0 || idx == string::npos) {
                return false;
            } else {
                // Syntaxe : substr(idx_from, nb_chars)
                key = line.substr(0, idx);
                string reste = line.substr(idx);
                val = U::trim_string(reste, ltrimchars, rtrimchars);
                return true;
            }
    }

    // Conversion de type en chaine de caractere pour affichage
    // template <class T>
    // static
    // std::string to_s(constT val) {
    //     std::ostringstream oss;
    //     oss << val;
    //     return oss.str();
    // }
    template <typename T>
    static
    std::string to_s(const T& val) {
        std::ostringstream oss;
        oss << val;
        return oss.str();
    }
    // template < typename T > std::string to_string( const T& n )
    // {
    //     std::ostringstream stm ;
    //     stm << n ;
    //     return stm.str() ;
    // }

    // Conversion d'une chaine en n'importe quoi --- JAMAIS TESTÉ !!
    // template <typename T>
    // static
    // T from_s(string str) {
    //     T output;
    //     istringstream in(str);
    //     in >> output;
    //     return output;
    // }

    // Utilitaire pour conversion de tableau (join, split, ...)
    // TODO :
    // std::vector<string>
    // split(string line, string chars, bool one_only)
    //
    /// static std::string
    /// join(vector<int> tab, std::string sep=", ") {
    ///     std::string the_sep = "";
    ///     std::ostringstream buf;
    ///     for (unsigned i=0; i<tab.size(); i++) {
    ///         buf << the_sep << tab[i];
    ///         the_sep = sep;
    ///     }
    ///     return buf.str();
    /// }
    //
    // Transforme en chaine n'importe quel conteneur c
    //  col : la collection (i.e. le conteneur) à transformer en chaine
    //  sep : chaine de séparation des élements du conteneur
    //
    // BUG : ne fonction que dans les conditions suivantes
    //  - col est un conteneur (e.g. vector, ..) et non pas un pointeur sur un
    //    conteneur
    //  - les éléments du conteneur doivent être affichables par l'opérateur <<
    //
    template<typename COL>
    static std::string join(const COL& col,  const std::string& sep = ", ") {

        std::string the_sep = "";
        std::ostringstream buf;

        // type const_iterator car "COL" est passé "const" (constant)
        typename COL::const_iterator it;
        for (it = col.begin(); it != col.end(); ++it) {
            buf << the_sep << *it;
            the_sep = sep;
        };
        return buf.str();
    }
    // Jamais testée
    void replace_all(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            size_t end_pos = start_pos + from.length();
            str.replace(start_pos, end_pos, to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }


};

#endif
//./
