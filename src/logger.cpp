// Ce fichier contiendra un classe Logger calquée sur java ou ruby.
// En attendant il n'y a que quelques méthodes d'aide à l'affichage
//

#include "logger.hpp"

////////////////////////////////////////////////////////////////////////
// La classe Log (minimaliste pour l'instant !)
//
Log::Log() {}
Log::~Log() {}
int  Log::level = 1 ;

// La fonction principale de logging : affiche une chaine si niveau suffisant
bool log(int level, string str) {
    if (str == "") {
        // chaine vide : on veut juste le test
        if (Log::level >= level) {
            return true;
        } else {
            return false;
        }
    }

    // Sinon, on affiche la chaine que si niveau suffisant
    if (Log::level >= level) {
        cout << "L" << level << ":" << str << flush;
        // cout << str;
        return true;
    } else {
        return false;
    }
}
bool log0  (string str)  { return log  (0, str); }
bool log1  (string str)  { return log  (1, str); }
bool log2  (string str)  { return log  (2, str); }
bool log3  (string str)  { return log  (3, str); }
bool log4  (string str)  { return log  (4, str); }
bool log5  (string str)  { return log  (5, str); }
bool log6  (string str)  { return log  (6, str); }
bool log7  (string str)  { return log  (7, str); }
bool log8  (string str)  { return log  (8, str); }
bool log9  (string str)  { return log  (9, str); }

// Version avec fin de ligne (n=newline)
bool logn(int level, string str) {
    string str2 = str.append("\n");
    return log(level, str2);
}
bool logn0 (string str)  { return logn (0, str); }
bool logn1 (string str)  { return logn (1, str); }
bool logn2 (string str)  { return logn (2, str); }
bool logn3 (string str)  { return logn (3, str); }
bool logn4 (string str)  { return logn (4, str); }
bool logn5 (string str)  { return logn (5, str); }
bool logn6 (string str)  { return logn (6, str); }
bool logn7 (string str)  { return logn (7, str); }
bool logn8 (string str)  { return logn (8, str); }
bool logn9 (string str)  { return logn (9, str); }


// Une fonction de test pour la mise au point des fonctions de log.
// Cette fonction peut aussi servir de documenetaion d'utilisation !
//
void _log_test(int level) {
    cout << "############################################\n"
         << "_log_test avec level=" << level << endl;
    int old_level = Log::level;
    Log::level= level;

    string s1 = "ma_chaine";
    string s2 = "ma_chaine";

    cout << "\n---- test le log(2, s1)" << endl;
    log(2,  s1);
    log(2,  "--CONCAT--");
    log(2,  s1);

    cout << "\n---- test le logn(2, s1)" << endl;
    logn(2,  s1);
    logn(2,  "--CONCAT (avec logn)--");
    logn(2,  s1);

    cout << "\n---- test le log3()" << endl;
    if (log3()) {
        cout << "On vient de test un log3()\n";
        cout << "Avec deux cout<< comme corps du if\n";
    }


    cout << "\n---- test le log1() et le to_s() PAS FAIT" << endl;
    logn1("a s1=" + s1);
    // logn1("b s1=" + s1 + 2); // PLANTE car 2 pas une string
    // logn1("s1=" + "s2="); // PLANTE aussi car deux "char*" autour d'un "+"
    logn1("b s1=" + s1 + " to_s(2)=" + to_s(2));
    logn1("c s1=" + s1 + " to_s(2)=" + to_s(2) + "  s2=" + s2);
    logn1("d s1=" + s1 + " to_s(2)=" + to_s(2) + "  s2=" + s2 + " double=" + to_s(123.012));
    cout << endl << "----- FIN --- " << endl;
    Log::level = old_level;

}

//./
