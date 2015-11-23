#ifndef LOGGER_HPP
#define LOGGER_HPP

// Ce fichier contiendra un classe Logger calquée sur java ou ruby.
// En attendant il n'y a que quelques méthodes d'aide à l'affichage.
//
// Les log permettent de gérer l'affichage de messages (pour information
// ou pour déboguer) de manière plus ou moins détaillée selon le niveau
// de verbosité choisi par le paramètre level (attribut de classe).
// Cet attribut peut par exemple être augmenté au début d'une
// fonction pour facilité sa mise au point.
//
// Cette fonctionnalité disponible dans la pluspart des langages
// évolués évite de passer son temps à commenter/décommenter des
// printf ou de cout<< pour le débogage.
//
// Exemples :
//
// Affichage d'un message (avec ou sans fin de ligne) si le niveau
// d'erreur est au moins 3
//
//    log3('Pas content, ');  // sans fin de ligne final
//    logn3('parce que...');  // avec fin de ligne final
//
// Effectue un traitement arbitraire si le niveau d'erreur est au moins 3
//
//    if (logn3()) {
//        // Construire mon_message qui peut-etre long ou couteux
//        cerr << mon_message;
//        // faire autre traitement particulier
//        exit(1);
//    }
//

#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

// Débug de classe pour les logging (affichage pout débohuer)
//
// Pour l'instant, seule l'attribut de classe Log::level est déclaré
//
class Log {
    // Cette classe n'est pas faite pour etre instanciée
    private: Log();
    private: ~Log();
    public: static int level;

    // ////////////////////////////////////////////////////////////////////////
    // // Fonction de conversion en string
    // //
    // template <class T>
    // static
    // std::string to_s(T val) {
    //     std::ostringstream oss;
    //     oss << val;
    //     return oss.str();
    // }

};

////////////////////////////////////////////////////////////////////////
// Fonction de conversion en string
// Attention : si la classe cliente dispose déjà de sa méthode to_s,
// elle devra appeler la fonction ci-dessous par ::to_s
// ATTENTION : DOUBLE EMPLOI AVEC CLASSE U (util.hpp)
//             voir aussi U::to_s(xxx)
template <class T>
std::string to_s(T val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}


////////////////////////////////////////////////////////////////////////
// Les fonctions standard de logging

// Pour des exemple : voir fonction _log_test
//
bool log(int verboseLevel, string str="");
bool log0(string str="");
bool log1(string str="");
bool log2(string str="");
bool log3(string str="");
bool log4(string str="");
bool log5(string str="");
bool log6(string str="");
bool log7(string str="");
bool log8(string str="");
bool log9(string str="");

bool logn(int verboseLevel, string str="");
bool logn0(string str="");
bool logn1(string str="");
bool logn2(string str="");
bool logn3(string str="");
bool logn4(string str="");
bool logn5(string str="");
bool logn6(string str="");
bool logn7(string str="");
bool logn8(string str="");
bool logn9(string str="");


void _log_test(int level=5);


#endif
//./
