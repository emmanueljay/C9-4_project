#!/usr/bin/env ruby
# encoding: utf-8

########################################################################
def get_version
  return "velib-v1.3.5-2015-12-03"
end
# 2015-12-03 : correction bug de lenteur (car construction d'un grosse chaine
#              pour debug avec la OpenStructure claimed, même si loglevel faible !
#              En effet une clé de claimed est "desequilibres" qui elle-même est
#              une Hash les les clés de des objets remorque.
#              Or l'affichage d'un objet remorque affiche tout la remorque
#              (dont le benche pour chaque remorque !!)
#              Deux pistes :
#              1. Une solution : ne pas afficher claimed (FONCTIONNE)
#              2. ou remplacer la clé rem par rem.name (FONCTIONNE AUSSI)
#              3. afficher séparément chaque élément de la hash (FONCTIONNE AUSSI)
# 2015-12-03 : correction bug de non vérif caa initiales des remorques
# 2015-11-10 : bug de coloration en rouge des arcs saturés
# 2015-11-09 : modif label légende
# 2015-11-06 : dépister bug d'efficacité avec ruby-2.x
#              et jruby plante car pb OutOfMemoryError
# 2015-11-05 : correction bug dans affichages statistiques
# 2015-10-22 : accept circuit en plus de tournee dans format de solution
# 2014-12-28 : l'option -p positionne automatiquement le flag --show_sol_pdf
# 2014-11-27 : correction bug (lignes YYY)
#              puts @nb_visits.to_s fait plante => JE COMMENTE !
#              EXLICATION identique à 2015-12-03 :
#              Ne pas afficher **directement** une hash ayant des clés complexe
#             (Station car contient un attribut Bench !!!)

def get_short_desc
    txt = <<-EOT.gsub(/^ {4}/,"")
    =======================================================================
    === #{get_version}
    === Générateur d'instance ou validateur de solution pour problème velib
    === Pour en savoir plus : velib -h
    =======================================================================
    EOT
end

def velib_full_doc
    exe = File.basename($0)
    # return "popopopo"
    # fullDoc = <<-EOT.gsub(/^    /,"")
    fullDoc = <<-EOT.gsub(/^ {4}/,"")
    Exemples : (à compléter)

    Tout savoir sur le fichier d'entrée \"mini.dat\" :
        #$0 -d mini.dat  --show_in_dists

    Tester une instance et une de ses solutions faisables
        #$0 -d mini.dat -s mini.sol --show_sol_detail
        #$0 -d mini.dat -s mini.sol --show_sol_detail \\
                    > mon_fichier.sol.detail
        #$0 -d mini.dat -s mini.sol \\
                --show_all > mini.sol.show_all

    Générer un fichier pdf pour une solution donnée (même si elle est fausse)
        #$0 -d mini.dat -s mini.sol --show_sol_detail --show_sol_pdf

    Convertit une instance au format OPL pour l'utiliser avec oplrun (CPLEX)
        #$0 -d mini.dat --show_in_opl > mini.opl.dat

    Générer un instance aléatoire sur la sortie standard
        #$0 -g --nb_stations 20  --nb_remorques 2  --capa_remorque 20 --capa_station 5..20 --seed 123
        #$0 -g -S20 -R2 -K20 -C 5..20 -r123

    Debug (pour le développeur de ce script Ruby)
        #$0 -L i     -d mini.dat -s mini.sol
        #$0 --testbench
        #$0 --testsol
        => pour autotest : fournit plein d'infos sur le bench mini

    Pour toutes infos complémentaires : Maurice.Diamantini@ensta-paristech.fr
    EOT
end
# #########################################################################
# COMPLEMENT DE DOC INTERNE
# #########################################################################
#
# HYP :
# - toutes le remorques générées ont la même capa (facile à changer)
#   On pourrait par exemple passer un interval -K 5..10 en plus d'une valeur
#   fixe comme -K 7
# - tous les points de localisation des sites sont différents
# - définition de vecteur charge[i] : contenu de la remorque **après** le
#   passage et dépot à la station s_i
# - pour la lecture des fichiers d'instance ou de solution, les valeurs des
#   sont convertit en flottant, puis en entier par arrondi.
#
# TODO :
# - reprendre les méthode Solution#update et Solution#validtate pour les
#   simplifier/factoriser/fusionner en séparant :
#   1. mise à jour des attributs (circuit, desequilibres, desequilibre, ...)
#   2. mise à jour de @valide et et @errmsg
#
# #########################################################################



# Quelques fonctions utilitaires génériques à déplacer
#
# Return true ssi exe est un exécutable accessible directement ou via la
# variable PATH
def exe_in_path?(exename)
    # On peut avoir fourni un chemin absolu ou relatif complet
    return true if File.executable?(exename)
    # On cherche dans la variable PATH
    ENV["PATH"].split(":").each do | path |
        return true if File.executable?(path + "/" + exename)
    end
    return false
end

########################################################################

#
# Exemple de fichier d'instance à analyser
#
#    velib instance_1 2 8
#    version 1.0
#
#    #remorque   id  x   y   K
#    remorque    r1  25  25  6
#    remorque    r2  75  75  6
#
#    #station  id     x   y  capa  ideal   nbvp
#    station   s1    10  50    11      8      8
#    station   s2    20  90     9      7      3
#    station   s3    25  60    10      7     10
#    station   s4    45  80     6      4      2
#    station   s5    40  60     7      5      5
#    station   s6    60  40     6      5      6
#    station   s7    80  50     7      5      5
#    station   s8    95  85     7      6      7
#
#
# Exemple de fichier d'instance au format OPL
#
#    name=instance_1;
#
#    #     id  x   y   K
#    remorques = {
#        < r1  25  25  6 >
#        < r2  75  75  6 >
#    };
#
#    #     id     x   y  capa  ideal   nbvp
#    stations = {
#        < s1    10  50    11      8      8 >
#        < s2    20  90     9      7      3 >
#        < s3    25  60    10      7     10 >
#        < s4    45  80     6      4      2 >
#        < s5    40  60     7      5      5 >
#        < s6    60  40     6      5      6 >
#        < s7    80  50     7      5      5 >
#        < s8    95  85     7      6      7 >
#    };
#
#
# Exemple de fichier solution
#
#   nom instance_1
#   desequilibre 0
#   distance 306
#
#   circuit r1 4 0 151
#   s5 0
#   s2 4
#   s3 -3
#   s1 0
#   end
#
#   circuit r2 4 0 155
#   s4 2
#   s6 -1
#   s7 0
#   s8 -1
#   end
#
########################################################################
require 'optparse'    # pour analyse de la ligne de commande
require 'ostruct'     # pour OpenStruct
# require 'Matrix'
require 'logger'      # getsion des niveaux de loggin (debug, ...)
require 'date'        # Pour manip plus intelligentes sur les dates
# require 'ostruct'   # OpenStruct
require 'pp'          # pretty-printer
require 'enumerator'  # pour methode Array.each_cons {...}

# Une variable globale pour deboguer :
# config du niveau du Logger (DEBUG INFO WARN ERROR FATAL ANY)
# $log.level = Logger::DEBUG
$log = Logger.new(STDOUT)
$log.level = Logger::WARN
def set_loglevel(str_level)
    case str_level
        when "debug" ; $log.level = Logger::DEBUG
        when "info"  ; $log.level = Logger::INFO
        when "warn"  ; $log.level = Logger::WARN
        when "error" ; $log.level = Logger::ERROR
        when "fatal" ; $log.level = Logger::FATAL
        else $log.level = Logger::DEBUG
    end
end


# Cette classe encapsule les options de l'appli, et se charge de l'analyse des
# arguments
#
class Args

    # Liste des noms d'options utilisées avec leur valeur par défaut
    def options_spec
        return {
            "datfile"           => nil,
            "solfile"           => nil,
            "pdffile"           => "visu_velib.pdf",

            "show_in_file"      => false,
            "show_in_detail"    => false,
            "show_in_dists"     => false,
            "show_in_opl"       => false,
            "show_sol_file"     => false,
            "show_sol_detail"   => false,
            "show_sol_pdf"      => false,
            "strict_sol_format" => true,

            "generate"          => false,
            "seed"              => 0,
            "nb_stations"       => 8,
            "nb_remorques"      => 2,
            "capa_remorque"     => 6,
            "capa_remorque_min" => 6,
            "capa_remorque_max" => 6,
            "xy_max"            => 100,
            "capa_station_min"  => 3,
            "capa_station_max"  => 10,

            "loglevel"          => Logger::WARN,
            "force"             => false,

            "testbench"         => false,
            "testsol"           => false,
            # "testtk"            => false,
        }
    end
    # Initialise ls options à leur valeur par défaut, pourrait être "reset_options"
    def init_options
        for name,default in options_spec

            # On évalue "attr_accessor" (privée) au niveau de la classe
            self.class.class_eval { attr_accessor name.intern }

            # on affecte la valeur initiale
            #   eval "@#{name.to_s} = #{init_val}"
            instance_variable_set( "@#{name}".intern, default )
        end
    end
    def trace_options
        for name,default in options_spec
            # Il semble que la commande ruby local_variable_set n'existe pas
            # contrairement à instance_variable_set, d'où ce hack !
            val = "__UNDEFIND__" # Doit être connue avant eval pour exister après (?)
            eval "val=@#{name}"
            default = "nil" if default == nil
            val = "nil" if val == nil
            puts name.to_s.ljust(20) + "= #{val}".ljust(15) + " (défaut=#{default})"
        end
    end


    def initialize(args)
        init_options

        # Définition des options et des handler associés
        #
        parser = OptionParser.new do |p|
            # p.program_name = File.basename $0
            p.banner = "velib.rb - génération d'instance ou vérification de solution " <<
                       "du problème Velib"
            p.separator "Version : #{get_version}"
            p.separator "Syntaxe : #$0 [option]"
            p.separator "\nOptions principales :"

            p.on('-d', '--datfile=FILE',
                 "Nom du fichier d'instance ")  { |val|
                @datfile = val
            }
            p.on('-s', '--solfile=FILE',
                 "Nom du fichier solution à valider")  {  |val|
                @solfile = val
            }

            p.on('-p', '--pdffile=FILE',
                 "Nom du fichier pdf à générer (défaut: #{@pdffile})")  {  |val|
                @pdffile = val
                @show_sol_pdf = true
            }

            p.separator "\nAffichage divers :"

            p.on( '--[no-]show_in_file',
                   "Affiche le fichier d'entrée regénéré format standard")  { |val|
                @show_in_file = val
            }
            p.on( '--[no-]show_in_detail',
                   "Affiche des info. détailés sur le fichier d'entrées (stats..)") { |val|
                @show_in_detail = val
            }
            p.on( '--[no-]show_in_opl',
                   "Affiche le fichier d'entrée regénéré format de données \"opl\"") { |val|
                @show_in_opl = val
            }
            p.on( '--[no-]show_in_dists',
                  "Affiche les distances de l'instance ")  { |val|
                @show_in_dists = val
            }
            p.on( '--[no-]show_sol_file',
                  "Affiche le fichier solution regénéré ")  { |val|
                @show_sol_file = val
            }
            p.on( '--[no-]show_sol_detail',
                  "Affiche les détails de la solution ")  { |val|
                @show_sol_detail = val
            }
            p.on( '--[no-]show_sol_pdf',
                  "génère un fichier pdf pour la solution ",
                  "(utilisable sans solution pour visualiser les sites")  { |val|
                @show_sol_pdf = val
            }
            p.on( '--show_none',
                  "N'affiche rien (ajouter explicitement ce qui vous souhaitez) ")  { |val|
                @show_in_file = false
                @show_in_detail = false
                @show_in_opl = false
                @show_in_dists = false
                @show_sol_file = false
                @show_sol_detail = false
                @show_sol_pdf = false
            }
            p.on( '--show_all',
                  "affiche toutes les infos sur l'instance et/ou la solution ")  { |val|
                @show_in_file = true
                @show_in_detail = true
                @show_in_opl = true
                @show_in_dists = true
                @show_sol_file = true
                @show_sol_detail = true
                @show_sol_pdf = true
            }
            p.on( '--[no-]strict_sol_format',
                  "respect strict du format de la solution ")  { |val|
                @strict_sol_format = val
            }
            p.on( '-u', '--no-strict_sol_format',
                  "souple au niveau du format de la solution ")  { |val|
                @strict_sol_format = val
            }

            p.separator ""
            p.separator "Génération d'instances aléatoires (options principales) :"

            p.on( '-g', '--[no-]gen', '--[no-]generate',
                  "Mode générateur d'instance. Vous pouvez aussi utiliser : ",
                  "  -d AUTO pour générer un nom de fichier automatiquement",
                  "  -d  -   pour imposer la sortie standard (défaut)")  { |val|
                @generate = val
            }
            # Attention, pour s'assurer que 007 ne soit pas pris pour de l'octal,
            # il faut passer par le type String et convertir par to_i .
            p.on( '-r', '--seed=N', String,
                 "Graine initialisant la génération (mettre 0 \"non déterministe\")")  { |val|
                @seed = val.to_i
            }
            # BUG DE OptionParser : il interprète "020" en octal contrairemenet à to_i
            # DONC : JE PASSE PAR LE TYPE String
            p.on( '-S', '--nb_stations=N', String,
                 "Nombre de stations à générer")  { |val|
                @nb_stations = val.to_i
            }
            p.on( '-R', '--nb_remorques=N', Integer,
                 "Nombre de remorques à générer")  { |val|
                @nb_remorques = val
            }
            # La capa peut être de la forme "5..10" ou "5"
            range_or_int_pat = /^(\d+)$|^(\d+)\.\.(\d+)$/
            p.on( '-K', '--capa_remorque=min..max', range_or_int_pat,
                   "capacité de chaque remorque")  { |tab|
                pp tab
                # =>  [full_str, capa, capa_min, capa_max]
                if tab[1]
                    @capa_remorque_min = tab[1].to_i
                    @capa_remorque_max = tab[1].to_i
                else
                    @capa_remorque_min = tab[2].to_i
                    @capa_remorque_max = tab[3].to_i
                end
                @capa_remorque = (@capa_remorque_min+@capa_remorque_max)/2
            }

            p.on( '-C', '--capa_station=min..max', range_or_int_pat,
                   "capacité de chaque station")  { |tab|
                pp tab
                # =>  [full_str, capa, capa_min, capa_max]
                if tab[1]
                    @capa_station_min = tab[1].to_i
                    @capa_station_max = tab[1].to_i
                else
                    @capa_station_min = tab[2].to_i
                    @capa_station_max = tab[3].to_i
                end
            }

            p.separator ""
            p.separator "Génération d'instances aléatoires (options secondaires) :"


            p.on( '--xy_max=N', Integer,
                   "Valeur maxi (exclue) pour les coordonnées x et y")  { |val|
                @xy_max = val
            }

            p.separator ""
            p.separator "Options réservées (pour test ou béboguage) :"

            p.on('--testtk',
                 "juste un test pour valider l'installation de ruby avec Tk") {
                                                                        | val |
                # Il ne faut pas lancer les tests ici car les exceptions
                # éventuelles  seraient masquées
                @testtk = 1
            }
            p.on( '--testbench',  "Autotest minimaliste de la classe Bench" ) {
                # Il ne faut pas lancer les tests ici car les exceptions
                # éventuelles  seraient masquées
                @testbench=1
            }
            p.on( '--testsol',  "Autotest minimaliste de la classe Solution" ) {
                # Il ne faut pas lancer les tests ici car les exceptions
                # éventuelles  seraient masquées
                @testsol=1
            }

            p.separator "\nOptions standard :"

            p.on('--version',
                 "affiche la version du programme et quitte ")    {
               puts get_short_desc
               exit
            }

            p.on( '-e', '--edit',  "Editer ce script lui même" ) {
                editor = ENV['EDITOR'] || "vim"
                unixCmd = "#{editor} #$0 &"
                puts "unixCmd=#{unixCmd}"
                system(unixCmd)
                exit
            }
            p.on( '-f', '--force',  "Force l'exécution (e.g. fichier existant...)" ) {
                @force = 1
            }
            p.on('-L', '--loglevel=LEVEL',
                  %w(debug info warn error fatal any),
                   "Niveau de verbosité (peut-etre abrégé à l'initiale)" ,
                   "(possible : debug info warn error fatal any) ") { |val|
                @loglevel = val
                set_loglevel(@loglevel)
                puts "loglevel.to_s=#{@loglevel.to_s}"
                puts "$log.level.to_s=#{$log.level.to_s}"
            }
            p.on_tail('--help', '-h', "Afficher les options disponibles") do
                puts "="*80
                puts p
                puts "="*80
                # raise "On arete !"
                exit
            end
            p.on_tail('--Help', '-H', "Afficher une aide complète") do
                puts "="*80
                puts p
                puts "="*80
                puts velib_full_doc()
                puts "="*80
                exit
            end
        end

        if args.empty?
            puts get_short_desc
            exit
        end

        # Analyse de la liste des arguments effectivement passés
        #
        # parser.parse(args)
        begin
            # la variante "parser.parse!(args)" vire les arguments lus.
            parser.parse(args)
        rescue SystemExit  => e
            # Cas particulier d'un exit : on n'affiche aucune aide.
            # puts "Sortie normale"
            exit 1
        rescue Exception => e
            puts parser.to_s
            # to_s affiche par exemple "invalid option: -m"
            puts "\n" + e.to_s + "\n\n"
            exit 1
        end
        # Exploitation directe de quelques options
        Bench.test if @testbench
        Solution.test if @testsol
        test_tk    if @testtk

        # Quelques vérif, ou post-traitement sur les options
        # (e.g. ohérence entre options...)
        # ...

        trace_options if $log.info?
    end
end #class Args

class Bench
    attr_accessor :name
    attr_accessor :stations, :remorques
    attr_accessor :bestSol, :log
    # attr_accessor :curSol  # non utilisé à l'exterieur pour l'instant

    # path: chemin d'acces au fichier d'instance
    # name: nom de l'instance. Par défaut, le programme en définit un en
    #       fonction du nom du fichier d'instance.
    #
    # args :
    #   path => "chemin de l'instance"
    #   name => nom de l'instance (prioritaire sur celui du fichier)
    #
    def initialize(path=nil)

        # liste des objets Station dans l'ordre de l'instance
        @stations = Array.new

        # liste des objets Remorque dans l'ordre de l'instance
        @remorques = Array.new

        # Nom de l'instance
        @name = nil

        # # La liste des arcs
        # @arcs = Array.new
        # La liste des arcs indicée par le tableau [site1,site2]
        @arcs_map = Hash.new

        # Ces deux circuits seront manipulés par les classes extérieures
        # (e.g. l'application mère ou les solveurs propriétaires)
        # @curSol = Solution.new self
        # @bestSol = Solution.new self
        @curSol = nil
        @bestSol = nil

        self.clear

        if path
            self.read(path)
        end
    end
    # Cette méthode publique de haut niveau gérera différents formats
    # automatiquement.
    # Pour l'instant, elle se contente d'appeler la méthode privée
    # read_velib_instance
    #
    def read(path)
        if path && File.exist?(path) && File.readable?(path)
            self.clear
            # On ne peut pas préfixer par "self" car la méthode est privée
            read_velib_instance(path)
        else
            puts path + " : Impossible de lire le fichier " + path
            exit
        end
    end
    # initialise ou réinitialise les attributs
    #
    def clear
        @stations.clear
        @remorques.clear
        # # @arcs.clear
        @arcs_map.clear
        # ces deux objets peuvent ne pas être initialisés
        @bestSol.clear unless ! @bestSol
        @curSol.clear  unless ! @curSol
        # @name = "NO_NAME"
        @name = nil # modif NO_NAME en nil pour goticvisu le 28/01/2010
    end
    # Fabrique de quelques instances de test
    def Bench.new_velib_mini
        b = Bench.new
        b.name = "instance_1"
        b.add_remorque "r1", 25, 25, 6
        b.add_remorque "r2", 75, 75, 6
        b.add_station "s1",  10, 50,  11,   8,  8
        b.add_station "s2",  20, 90,   9,   7,  3
        b.add_station "s3",  25, 60,  10,   7, 10
        b.add_station "s4",  45, 80,   6,   4,  2
        b.add_station "s5",  40, 60,   7,   5,  5
        b.add_station "s6",  60, 40,   6,   5,  6
        b.add_station "s7",  80, 50,   7,   5,  5
        b.add_station "s8",  95, 85,   7,   6,  7
        return b
    end

    # génère une instance en fonction des options
    #
    # Principe :
    # On commence par générer des stations equilibrées en fonction des options.
    # (on a donc s.nbvp := s.ideal  forall s in stations)
    # Puis on provoque des mouvements aléatoires (mais valide) de nbv vélos entre
    # chaque arc s1->s2  avec s1<s2 mais nbv pouvant être négatif (pour éviter de
    # traiter deux fois une arête)
    #
    # Le problème est donc toujours équilibré **si la remorque est assez grande**
    #
    # TODO : on pourra si nécessaire similer le vol de vélo par un traitemenet
    # postérieurs sur les stations non vide
    # (prévoir alors une option --vol=INT)
    #
    #
    def Bench.generate opts

        # Hash<[x,y]> de site.name  assurant l'unicité des positions (x,y) des sites
        points = Hash.new

        # Construction de l'instance
        b = Bench.new

        if opts.seed != 0
            # Une valeur non nulle de la seed permettra de regénérer la même instance
            # (sous réserve de meme machine virtuelle ruby ?)
            # Sans cette instruction, le générateur sera aléatoire.
            srand opts.seed
        end

        # Le nom de l'instance générée est de la forme
        #   velib_s20_k20.dat  pour une instance aléatoire
        #   velib_s20_r2_k20_seed222.dat pour une instance déterministe (avec seed)
        if opts.datfile && opts.datfile != "-" && opts.datfile != "AUTO"
            b.name = File.basename(opts.datfile, ".dat")
        else
            b.name = "velib_s#{opts.nb_stations}"
            b.name << "_r#{opts.nb_remorques}"      if opts.nb_remorques != 1
            b.name << "_k#{opts.capa_remorque}"
            b.name << "_seed#{opts.seed}"           if opts.seed != 0
        end

        # puts "TEST RAND: rand(100)=" + rand(100).to_s
        # puts "TEST RAND: rand()=" + rand().to_s
        # puts "opts.cmp_proba=#{opts.cmp_proba}"

        # Génération des remorques
        #
        for i in 1..opts.nb_remorques
            name = "r#{i}"
            begin
                x = rand opts.xy_max
                y = rand opts.xy_max
            end while points.include? [x,y]
            points[[x,y]] = name

            # b.add_remorque name, x, y, opts.capa_remorque # OLD GOOD 13/11/2014

            # Calcul de capa de la remorque.
            #
            capa = opts.capa_remorque_min +
                   rand(1 + opts.capa_remorque_max - opts.capa_remorque_min)

            b.add_remorque name, x, y, capa
        end

        # Génération des stations equilibrées
        #
        for i in 1..opts.nb_stations
            name = "s#{i}"
            has_collision = false  # Juste pour tester nb colisions sur grosses instances
            begin
                x = rand opts.xy_max
                y = rand opts.xy_max
                $log.info "Collision dans calcul xy pour #{name}" if has_collision
                has_collision = true
            end while points.include? [x,y]
            points[[x,y]] = name

            # Calcul de capa de la station.
            #
            capa = opts.capa_station_min + rand(1 + opts.capa_station_max - opts.capa_station_min)

            # puts "opts.capa_station_min=#{opts.capa_station_min}"
            # puts "opts.capa_station_max=#{opts.capa_station_max}"
            # puts "#{name} : capa=#{capa}"

            # Calcul du ideal pour la station.
            #
            ideal = rand(1 + capa)

            # Calcul de nbvp dans la station : équilibrée !
            #
            #### nbvp = rand(1 + capa)
            nbvp = ideal

            st = b.add_station name, x, y, capa, ideal, nbvp
        end
        for arc in b.arcs
            next if arc.src.is_a?(Remorque) || arc.dst.is_a?(Remorque)
            arc.move!
        end
        return b
    end

    # ajout et retourne une nouvelle remorque dans le bench
    #
    # Les parametres sont des chaines de caractère !
    #
    def add_remorque(name, x, y, capa)
        rem =  Remorque.new(self, name, x, y, capa)
        build_arcs_for rem
        @remorques << rem
        return rem
    end

    # ajoute et retourne une nouvelle station dans le bench
    #
    # Les parametres sont des chaines de caractère !
    #
    def add_station(name, x, y, capa, ideal, nbvp)
        station =  Station.new(self, name, x, y, capa, ideal, nbvp)
        # Création des tout arc possible **avant ajout de cette nouvelle station**
        build_arcs_for station
        @stations << station
        return station
    end

    # Lit l'instance.
    # HYP: le fichier associé à datfile existe
    #
    # Pour l'instance on ne support aue le format suivant :
    # - l'id des stations est une chaine arbitraire
    # - Exemple :
    #
    #    velib instance_1 2 8
    #    version 1.0
    #
    #    #remorque   id  x   y   K
    #    remorque    r1  25  25  6
    #    remorque    r2  75  75  6
    #
    #    #station  id     x   y  capa  ideal   nbvp
    #    station   s1    10  50    11      8      8
    #    station   s2    20  90     9      7      3
    #    station   s3    25  60    10      7     10
    #    station   s4    45  80     6      4      2
    #    station   s5    40  60     7      5      5
    #    station   s6    60  40     6      5      6
    #    station   s7    80  50     7      5      5
    #    station   s8    95  85     7      6      7
    #
    private
    def read_velib_instance(datfile)

        lines = IO.readlines(datfile)
        lines.each_index do |line_idx|
            line = lines[line_idx]
            $log.debug("Début trait. de la line_#{line_idx}='#{line.chomp}'")

            # Ignore lignes  contenant un commentaire ou que des espaces
            next if  line =~ /^(\s*|\s*#.*)$/
            # on vire les espaces sur les bords
            line.strip!

            # On extrait le mot clé et les données
            # KEYWORD val1 ....  val3 val4 val4  # des commentaires
            # END  # un mot clé **sans** données associées
            #
            #   tab[0] la chaine (i.e. ligne) complète
            #   tab[1] mot clé (ou id d'une station)
            #   tab[2] la chaine formant les données
            #   tab[3] les commentaires (dont le caractère dièse)
            #
            tab = /^(\S+)(?:\s+([^#]+))?(.*)$/.match line
            if !tab
                msg = "ERREUR : format de ligne nom reconnue\n" <<
                      "line_#{line_idx}: '#{line}"
                puts msg
                exit
            end
            key, data, comment = tab[1,3]
            data.strip!    if data
            comment.strip! if comment

            $log.debug "key='#{key}' data='#{data}' comment='#{comment}'"
            case key.downcase
            when "velib"
                # nom_instance  nb_rem  nb_stations
                # On ignore les valeurs de  nb_rem et nb_sta
                tab = data.split(/\s+/)
                @name = tab[0]
                # @nb_remorques = tab[1]
                # @nb_station= tab[2]
                $log.debug "    => @name=#{@name} "
                next
            when "version"
                format_version = data.to_f
                if format_version != 1.0
                    puts "Version non supportée pour l'instant : #{format_version}"
                    puts "Seule le format de version 1.0 est reconnu."
                    exit 1
                end
                $log.debug "    => format_version=#{@format_version}"
                next
            when "remorque"
                # id  x   y   capa
                tab = data.split(/\s+/)
                if tab.size != 4
                    puts "Nombre incorrect de données pour la remorque :\n" <<
                          "line_#{line_idx}: '#{line}"
                    exit
                end
                name  = tab[0]
                x     = tab[1].to_f.round
                y     = tab[2].to_f.round
                capa  = tab[3].to_f.round
                self.add_remorque(name, x, y, capa)
                $log.debug "    => Lecture de la remorque #{name}"
                next
            when "station"
                # id     x   y  capa  ideal   nbvp
                tab = data.split(/\s+/)
                if tab.size != 6
                    puts "Nombre incorrect de données pour la station :\n" <<
                          "line_#{line_idx}: '#{line}"
                    exit
                end
                name  = tab[0]
                x     = tab[1].to_f.round
                y     = tab[2].to_f.round
                capa  = tab[3].to_f.round
                ideal = tab[4].to_f.round
                nbvp  = tab[5].to_f.round

                self.add_station(name, x, y, capa, ideal, nbvp)
                $log.debug "    => Lecture de la station #{name}"
                next
            else
                puts "Mot clé \"#{key}\" non reconnu dans la ligne :\n" <<
                      "line_#{line_idx}: '#{line}"
                exit

            end

        end # lines.each

    end # read_velib_instance
    public # remise à public

    # distance entre deux sites
    def dist(s1, s2)
        return Math.hypot(s1.x-s2.x, s1.y-s2.y).round
    end
    # Crée la liste de tous les arcs possibles (directs et inverses)
    # entre ce nouveau site et tous les sites existants.
    # Mais on n'ajoute jamais d'arc entre deux Remorques !
    #
    # HYPOTHÈSE :
    #   new_site n'est pas encore enregistré, ni dans @stations, ni dans
    #   @remorques (sinon il faudrait s'assurer de ne pas créer de boucles !)
    #
    def build_arcs_for  new_site
        connectable_sites = @stations.dup
        connectable_sites += @remorques unless new_site.is_a? Remorque
        for site in connectable_sites
            arc1 = Arc.new site, new_site
            @arcs_map[[site, new_site]] = arc1

            arc2 = Arc.new new_site, site
            @arcs_map[[new_site, site]] = arc2
        end
        ## for site in @stations + @remorques
        ##     arc1 = Arc.new site, new_site
        ##     @arcs_map[[site, new_site]] = arc1
        ##
        ##     arc2 = Arc.new new_site, site
        ##     @arcs_map[[new_site, site]] = arc2
        ## end
    end
    def arcs
        @arcs_map.values
    end
    def in_arcs dst
        arcs=Array.new
        @arcs_map.each do | key, arc |
            arcs << arc unless arc.dst != dst
        end
        return arcs
    end
    def out_arcs src
        arcs=Array.new
        @arcs_map.each do | key, arc |
            arcs << arc unless arc.src != src
        end
        return arcs
    end
    def get_arc(s1, s2)
        return @arcs_map[[s1, s2]]
    end

    # Affiche le bench dans un format défini par mode
    #    mode=std : affichage dans le format d'entrée
    #    mode=opl : sortie au format opl
    #
    def to_s(mode="std")
        case mode
        when "opl"
            return to_s_opl
        when "pretty"
            raise "Bench#to_s_pretty non implémentée"
            # return to_s_pretty
        end

        txt = "velib #{name} #{@remorques.size} #{@stations.size}\n" <<
              "version 1.0\n\n" <<
              "#remorque id     x   y   capa\n"

        for remorque in @remorques
            txt << remorque.to_s_full(mode) << "\n"
        end

        txt << "\n"
        txt << "#station id     x   y   capa ideal  nbvp\n"
        for station in @stations
            txt << station.to_s_full(mode) << "\n"
        end
        txt << "\n"
        return txt
    end
    def to_s_opl
        txt = "// Instance OPL pour projet velib version 1.0  vim:ft=c\n"
        txt << "nom = #{name};\n\n"

        txt << "//    <   id     x   y    capa >\n" <<
              "remorques = {\n"

        for r in @remorques
            txt << "      < " <<
                   r.name.rjust(4) << "  " <<
                   r.x.to_s.rjust(4) <<
                   r.y.to_s.rjust(4) << "  " <<
                   r.capa.to_s.rjust(6) <<
                   " >\n"
        end
        txt << "};\n"
        txt << "\n"

        txt << "//    <   id     x   y   capa ideal  nbvp >\n" <<
              "stations = {\n"

        for st in @stations
            txt << "      < " <<
                   st.name.rjust(4) << "  " <<
                   st.x.to_s.rjust(4) <<
                   st.y.to_s.rjust(4) << "  " <<
                   st.capa.to_s.rjust(5) <<
                   st.ideal.to_s.rjust(6) <<
                   st.nbvp.to_s.rjust(6) <<
                   " >\n"
        end
        txt << "};\n"
        txt << "\n"

        return txt
    end

    # Affiche les détails de l'instance (statistiques...)
    def to_s_detail()
        # Préparation de quelques statistiques ou attributs dérivés
        total_ideal = self.stations.reduce(0){ | total, station | total += station.ideal}
        total_nbvp = self.stations.reduce(0){ | total, station | total += station.nbvp}
        total_missing = total_ideal - total_nbvp

        max_arc = arcs.max{|arc1, arc2| arc1.dist <=> arc2.dist}
        min_arc = arcs.min{|arc1, arc2| arc1.dist <=> arc2.dist}

        cap_min = self.stations.min{|s1, s2| s1.capa <=> s2.capa}.capa
        cap_max = self.stations.max{|s1, s2| s1.capa <=> s2.capa}.capa
        cap_range =  "#{cap_min}..#{cap_max}"

        rem_cap_min = self.remorques.min{|r1, r2| r1.capa <=> r2.capa}.capa
        rem_cap_max = self.remorques.max{|r1, r2| r1.capa <=> r2.capa}.capa
        rem_cap_range =  "#{rem_cap_min}..#{rem_cap_max}"

        ideal_min = self.stations.min{|s1, s2| s1.ideal <=> s2.ideal}.ideal
        ideal_max = self.stations.max{|s1, s2| s1.ideal <=> s2.ideal}.ideal
        ideal_range =  "#{ideal_min}..#{ideal_max}"

        nbvp_min = self.stations.min{|s1, s2| s1.nbvp <=> s2.nbvp}.nbvp
        nbvp_max = self.stations.max{|s1, s2| s1.nbvp <=> s2.nbvp}.nbvp
        nbvp_range =  "#{nbvp_min}..#{nbvp_max}"

        deficit_min = self.stations.min{|s1, s2| s1.deficit <=> s2.deficit}.deficit
        deficit_max = self.stations.max{|s1, s2| s1.deficit <=> s2.deficit}.deficit
        deficit_range =  "#{deficit_min}..#{deficit_max}"

        txt =  "="*70  +  "\n"
        txt << "Details de l'instance de nom '#{self.name}'\n"

        txt <<  "Liste des stations (#{@stations.size})\n"
        for station in @stations
            txt << station.to_s_pretty() + "\n"
        end
        txt << "\n"

        txt <<  "Liste des remorques (#{@remorques.size})\n"
        for rem in @remorques
            ## txt << rem.name.ljust(4)  + " : "
            ## txt << "xy= " + rem.x.to_s.rjust(3) + rem.y.to_s.rjust(3) + "\n"
            txt << rem.to_s_pretty() + "\n"
        end
        txt << "\n"

        txt << "nom d'instance: #{name}\n"
        txt << "nombre de stations: #{@stations.size}  "
        txt << "nombre de remorques: #{@remorques.size}  "

        txt << "\n"
        txt << "Total vélo nécessaires (cumul de ideal):      #{total_ideal.to_s.rjust 3}\n"
        txt << "Total vélo présents (cumul de nbvp):          #{total_nbvp.to_s.rjust 3}\n"
        txt << "Nombre total de vélo manquants (e.g. volées)  #{total_missing.to_s.rjust 3}\n"
        if (total_missing.abs >= rem_cap_max)
            txt << "    => dépasse la capacité de la plus grosse remorque (#{rem_cap_max})"
        end
        txt << "\n"
        txt << "Statistiques des capacités des remorques :       #{rem_cap_range}\n"
        txt << "Statistiques des capacités des stations :        #{cap_range}\n"
        txt << "Statistiques des ideals des stations :           #{ideal_range}\n"
        txt << "Statistiques des vélo présents en stations :     #{nbvp_range}\n"
        txt << "Statistiques des deficits initiaux des stations :#{deficit_range}\n"
        txt << "\n"
        txt << "Distances des sites les plus éloignés :  "
        txt << "#{max_arc.src}<-->#{max_arc.dst} = #{max_arc.dist.to_s.rjust 3} Km\n"
        txt << "Distances des sites les plus proches :   "
        txt << "#{min_arc.src}<-->#{min_arc.dst} = #{min_arc.dist.to_s.rjust 3} Km\n"
        txt << "\n"
        # txt << self.dists_to_s
        return txt
    end

    # Affiche une chaine de la forme :
    #   ...
    #   S_1
    #       ->S_6  :  51km
    #       ->M_1  :  78km
    #       ...
    #
    #  ATTENTION : les arcs indiqués ne sont pas forcément réalisables
    #              (suivant timing)
    #
    def dists_to_s()
        txt = "\nAffichage des distances et des arcs possibles \n"
        for s1 in (self.remorques + self.stations)
            txt << s1.name + "\n"
            for s2 in (self.remorques + self.stations)
                arc = get_arc(s1,s2)

                # Si l'arc existe, on utilise les valeurs mémorisées, sinon on
                # les recalcule.
                dist  = arc ? arc.dist  : self.dist(s1,s2)
                txt <<  "     ->" <<
                      "#{s2.name.ljust(7)}: " <<
                      dist.to_s.rjust(3) <<
                      "km "

                if arc
                    txt << "\n"
                else
                    txt << " ARC IMPOSSIBLE (non fait)\n"
                end
            end
            txt << "\n"
        end
        return txt
    end
    # recherche un site (objet Station ou Remorque) à partir de son nom
    # retourne l'objet trouvé, ou nil si inexistant.
    #
    # Performance : O(n), mais pas critique car n'est utilisée que pour les
    #               entrée-sorties ou les tests
    #
    # HINT : si le nom est un entier, cette méthode s'assure de la convertion
    # en String ce qui permet de simplifier l'utilisation en écrivant "
    #    st = b.find{3)
    # au lieu de :
    #    st = b.find("3")
    #
    def find(name)
        for site in @stations + @remorques
            return site if (site.name.to_s == name)
        end
        return nil
    end

    # Génère un fichier un format .dot pour dessiner le graphe de l'instance
    # et éventuellement de la solution (si sol existe)
    #
    def get_dot(sol=nil)
        # nscale=1.0/20 * Math.sqrt(@stations.size).round
        nscale=1.0/35 * Math.sqrt(@stations.size).round
        txt = "digraph G {\n"

        # shape: ellipse, point, box, plaintext
        txt << %Q(  size="5,5";\n)
        txt << "  node [shape=circle, width=0.8, fontname=Helvetica];\n"
        # txt << "  node [shape=plaintext];\n"
        txt << %Q(  edge [color=blue];\n)
        txt << %Q(  outputorder=nodesfirst;\n)

        # On ajoute deux noeuds invisibles pour imposer la taille de graphe
        opts = '" label="" shape=none color=red '
        txt << "  BL__ [pos=\"#{ -5*nscale},#{ -5*nscale}! #{opts}];\n"
        txt << "  TP__ [pos=\"#{105*nscale},#{120*nscale}! #{opts}];\n"

        # Ajout de quelques pseudo-noeuds pour les labels du graphe
        label1 = %Q(Instance=#{@name} )
        if sol
            label1 << %Q( dist=#{sol.length} desequilibre=#{sol.desequilibre})
        end
        opts = %Q(label="#{label1}"
                  labeljust=l shape=none
                  color=navy  fontcolor=navy  fontsize=#{150*nscale})
        txt << %Q(  LABEL1 [pos="#{ 50*nscale},#{ 108*nscale}!" #{opts}];\n )

        # label2 = %Q(légende station: 'nom:besoin' ; légende remorque 'nom (<= capa_max)')
        # if sol
        #     label2 << %Q( ; légende arc: '-depos => charge remorque')
        # end
        label2 = %Q(légende : 'station:besoin' ; 'remorque (<= capa_max)')
        if sol
            label2 << %Q( ; arc: '-depos => nouvelle charge')
        end
        opts = %Q(label="#{label2}"
                  labeljust=l shape=none
                  color=navy  fontcolor=navy  fontsize=#{150*nscale})
        txt << %Q(  LABEL2 [pos="#{ 50*nscale},#{ 105*nscale}!" #{opts}];\n )

        if sol
            label3 = %Q(Arc vert => remorque vide ;       arc rouge => remorque plein)
            opts = %Q(label="#{label3}"
                      labeljust=l shape=none
                      color=navy  fontcolor=navy  fontsize=#{150*nscale})
            txt << %Q(  LABEL3 [pos="#{ 50*nscale},#{ 102*nscale}!" #{opts}];\n )
        end



        # construction des sommets (remorques et stations)
        # for rem in @remorques
        #     nodetxt =  %Q( %4s [pos="% .4g,% .4g!" shape=Msquare];\n) %
        #                 [ rem.name, nscale*rem.x.round(), nscale*rem.y.round()]
        #     txt << nodetxt
        # end
        for rem in @remorques
            nodetxt =  %Q( %4s [pos="% .4g,% .4g!" shape=Msquare label="%s (<=%s)"];\n) %
                        [ rem.name, nscale*rem.x.round(), nscale*rem.y.round(),
                         rem.name, rem.capa]
            txt << nodetxt
        end
        for st in @stations
            nodetxt =  %Q( %4s [pos="% .4g,% .4g!" label="%s: %s"];\n) %
                        [ st.name, nscale*st.x.round(), nscale*st.y.round(),
                            st.name, (st.ideal-st.nbvp)]
            # nodetxt =  %Q( %4s [pos="% .4g,% .4g!" label="%s\n%s=%s"];\n) %
            #             [ st.name, nscale*st.x.round(), nscale*st.y.round(), ...]
            txt << nodetxt
        end


        # Les arcs seulement si une solution est passée en paramètre
        if sol
            for rem, circuit in sol.circuits
                for arc in circuit.arcs

                    dep = circuit.depots[arc.src]
                    chg = circuit.charges[arc.src]

                    opts = %Q(fontcolor=darkgreen decorate=0)
                    if arc.src.is_a? Remorque
                        label = %Q(init=>#{chg})
                    else
                        # Le label sur l'arc est de la forme
                        #    -5 => 9
                        # si la remorque vient de déposer 5 vélos.
                        ### dep = circuit.depots[arc.src]
                        strdep = dep>0 ? "-#{dep.abs}" : "+#{dep.abs}"
                        label = %Q(#{strdep} => #{chg})
                    end
                    # Si la remorque est pleine : on double l'épaisseur de l'arc
                    # Si la remorque est vide : on dessine l'arc en pointillé.
                    #
                    ### opts << %Q( color=red style=bold)    if chg == @capa_remorque
                    opts << %Q( color=red style=bold)    if chg == rem.capa
                    opts << %Q( color=green2 style=bold)  if chg == 0


                    opts << %Q( taillabel="#{label}")
                    txt << %Q(  #{arc.src.name} -> #{arc.dst.name} [#{opts}];\n)
                end
            end
        end
        txt << "}\n"
        # puts txt;exit
    end
    def write_pdf(filename, sol=nil)
        for exe in %w(neato ps2pdf)
            if ! exe_in_path?(exe)
                STDERR.puts "impossible de trouver l'exécutable \"#{exe}\": abondon !"
                # exit
                return false
            end
        end
        dottxt = self.get_dot(sol)
        rootname = File.basename(filename, File.extname(filename))
        dirname  = File.dirname(filename)
        fullbasename = "#{dirname}/#{rootname}"
        ### puts "filename=#{filename}"
        ### puts "rootname=#{rootname}"
        ### puts "dirname=#{dirname}"
        ### puts "fullbasename=#{fullbasename}"
        ### exit
        dotfilename = "#{fullbasename}.dot"
        File.open(dotfilename, "w") { |fid| fid.write dottxt }
        cmd = "neato -Tps2 #{fullbasename}.dot | ps2pdf - #{fullbasename}.pdf"
        system cmd
        okstatus = $?.success?
        if okstatus
            File.delete dotfilename
        end
        return okstatus
    end

    def Bench.test()
        b = Bench.new_velib_mini
        puts b.to_s
        puts b.to_s "opl"

        puts "\nTest de b.out_arcs(b.find(\"r2\"))\n"
        for arc in b.out_arcs(b.find("r2"))
            puts "  " + arc.name
        end
        puts "\nTest de b.in_arcs(b.find(\"s2\"))\n"
        for arc in b.in_arcs(b.find("s2"))
            puts "  " + arc.name
        end
        exit
    end
end #class Bench

# Un Site décrit un lieu géographique (classe mère pour Tic et Job)
# Un site représente soit une intervention réelle (Job) soit une intervention
# virtuelle définie par un Tic (début et fin d'activité)
#
#
#  bench : l'instance propriétaire
#  name : un nom unique pour tout objet (e.g; tic_2 et job_4)
#  x, y : les coordonnées doivent pouvoir être converties en entier
#
#
class Site
    attr_reader :name, :x, :y, :b
    attr_reader :hash

    def initialize(bench, name, x, y)
        @b = bench
        @name = name.to_s
        @x = x.to_f.round
        @y = y.to_f.round
        @hash = @name.hash
    end

    # Retourne le hashcode identifiant l'objet. Cet entier est basé sur le
    # l'attribut @name qui ne doit donc plus être modifié.
    def hash
        @hash
    end

    # Affichage d'un Site
    def to_s
        @name
    end

    # mode : caracterise le format de sortie :
    # - std : identique au format d'entrée
    # - pretty : version présentée différemment (plus verbeuse)
    #
    def to_s_full(mode="std")
        txt = "station  "
        case mode
            when "std"
                # format standard de l'instance
                txt =  "#{@name.ljust 4}"   <<
                       " #{@x.to_s.rjust 3}" <<
                       " #{@y.to_s.rjust 3}"
            when "pretty"
                txt << to_s_pretty
            else
                raise "mode d'affichage inconnu \"#{mode}\""
        end
        return txt
    end
    def to_s_pretty()
        txt = ""
        txt << "  name: #{@name.to_s.rjust 3}" <<
               "   id: #{@id.to_s.rjust 3}" <<
               "   xy: #{@x.to_s.rjust 2}, #{@y.to_s.rjust 2}" <<
               "   capa: #{@capa.to_s.rjust 2}"
        return txt
    end
    # Operateur de comparaison sur le nom (pour le tri par exemple)
    def <=> other
        @name <=> other.name
    end
    # Calcul de la distance entre deux sites
    # Ce calcul de distance est sous-traité au Bench (qui mémorise en cache)
    #
    def dist_to(site2)
        @b.dist(self, site2)
    end
end #class Site

class Station < Site
    attr_reader :name, :id, :x, :y, :capa, :ideal
    # nbpv doit pouvoir être modifier par les Bench#generate pour provoquer un déséquilibrage
    attr_accessor :nbvp

    # @id représente le numéro d'ordre de création de l'objet
    @@next_id = 1
    def initialize(bench, name, x, y, capa, ideal, nbvp)
        @id = @@next_id
        @@next_id += 1
        super(bench, name, x, y)
        @capa = capa.to_f.round
        @ideal = ideal.to_f.round
        @nbvp = nbvp.to_f.round
    end

    def marge
        @capa - @nbvp
    end

    def deficit
        # @nbvp - @capa
        @ideal - @nbvp
    end

    # mode d'affichage : idem class Site
    def to_s_full(mode="std")
        txt = "station  "
        case mode
            when "std"
                # format standard de l'instance
                txt << "#{super(mode)}" <<
                      " #{@capa.to_s.rjust 6}" <<
                      "#{@ideal.to_s.rjust 6}" <<
                      "#{@nbvp.to_s.rjust 6}"
            when "pretty"
                txt << to_s_pretty
            else
                raise "mode d'affichage inconnu \"#{mode}\""
        end
        return txt
    end

    # Sortie complète en format plus libre
    def to_s_pretty()
        txt = ""
        txt << "  name: #{@name.to_s.rjust 3}" <<
               "   id: #{@id.to_s.rjust 3}" <<
               "   xy: #{@x.to_s.rjust 2}, #{@y.to_s.rjust 2}" <<
               "   capa: #{@capa.to_s.rjust 2}" <<
               "   ideal: #{@ideal.to_s.rjust 2}" <<
               "   nbvp: #{@nbvp.to_s.rjust 2}" <<
               "   deficit: #{deficit.to_s.rjust 3}"
        return txt
    end
end #class Tic

class Remorque < Site
    attr_reader :name, :id, :x, :y, :capa

    # @id représente le numéro d'ordre de création de l'objet
    @@next_id = 1
    def initialize(bench, name, x, y, capa)
        @id = @@next_id
        @@next_id += 1
        super(bench, name, x, y)
        @capa = capa
    end

    def to_s_full(mode="std")
        txt = "remorque  " # + super(mode).ljust(18)
        case mode
            when "std"
                # format standard de l'instance
                txt << "#{super(mode)}" <<
                       "#{@capa.to_s.rjust 7}"
            else
                # format plus libre
                txt << "name=#{@name.to_s.ljust 8} " <<
                       "id=#{@id.to_s.ljust 6}" <<
                       "#{@x.to_s.rjust 4}," <<
                       "#{@y.to_s.ljust 4}  " <<
                       "#{@capa.to_s.ljust 3}"
        end
        return txt
    end
end #class Job

# Un arc est essentiellement un couple d'objets Site
# - src : le site source
# - dst : le site destination
#
# et quelques méthodes d'acces :
# - dist : distance en km
# - move! : impose un déplacement (relatif) de vélo de la src vers la dest
#
# Un arc a acces au bench via l'un des deux sites src ou dst
class Arc
    attr_reader :src, :dst, :dist, :delay
    def initialize src, dst
        @src = src
        @dst = dst
        @dist = @src.b.dist(@src, @dst)
        @hash = to_s.hash
    end
    def to_s
        return @src.name + "->" + @dst.name
    end
    def to_s_full
        return "arc  " + @src.to_s_pretty + "\n   ->" + @dst.to_s_pretty
    end
    def name
        return self.to_s
    end
    # Retourne le hashcode identifiant l'objet. Cet entier est basé sur le
    # l'attribut @name qui ne doit donc plus être modifié.
    def hash
        @hash
    end

    # Déplace nbv vélos de la source vers la destination
    # Retourne le nombre de vélo transférés de src vers dst
    #
    # nbv: nombre (positif ou négatif) de vélos à déplacer.
    # nbv est éventuellement tronqué pour respecter les limites des sites
    # en tenant compte de leur nbvp et marge respective.
    # par défaut, nbv est pris aléatoiremenet dans les limites autorisées.
    #
    #
    def move!(nbv=nil)
        max_n =  [@src.nbvp, @dst.marge].min
        min_n = -[@dst.nbvp, @src.marge].min
        if nbv
            # nbv = [ [min_n,nbv].max , max_n ].min
            nbv = nbv<min_n ? min_n : nbv>max_n ? max_n : nbv
        else
            nbv = min_n + rand(max_n+1-min_n)
        end
        @src.nbvp -= nbv
        @dst.nbvp += nbv
        return nbv
    end

    # Retourne true ssi le passage d'un site à un autre est possible
    # compte tenu : (A COMPLETER)
    #
    def feasible
        # pour l'instant : pas de filtrage : on autorise tous les arcs.
        return true
    end
end #class Arc


# Simple création d'un bouton avec "quitter" comme action
def test_tk
    require 'tk'
    root = TkRoot.new { title "Test de Tk sous Ruby" }
    TkLabel.new(root) {
        text  'Essai de Ruby/Tk !'
        pack  { padx 15 ; pady 15; side 'left' }
    }
    TkButton.new(root) {
        text  'Quitter'
        pack  { padx 15 ; pady 15; side 'bottom' }
        command 'exit'
    }
    Tk.mainloop
end #def test_tk

# Un circuit représente le résultat d'une tournée associée à une remorque donnée.
# Les principaux attributs sont :
#   - b : le bench propriétaire
# Attributs primaires (indispensables pour construire une solution)
#   - remorque : la remorque correspondant à ce circuit,
#   - charge_init : charge initiale de la remorque
#   - tournees_data : tableau de paires [station, depot]
#     (n'est PAS mémorisé en attribut)
# Attributs dérivés (calculés à partir des attributs primaires)
#   - stations : la liste des stations gérées par cette remorque
#   - depots[Station] : la Hash des dépots des stations visitées
#   - arcs : liste des arcs représentant le chemin (plus pratique à exploiter
#     que la liste des stations visitées)
#
class Circuit
    # Pour l'instant, un Circuit n'est pas intelligent : tout est piloté par la
    # classe Solution (qui doit donc avoir un accès complet à tous les attributs)
    attr_accessor :remorque, :charge_init, :stations, :depots
    attr_accessor :charges, :deficits, :desequilibre , :nb_visits, :length
    attr_reader :valid, :errmsg, :arcs

    def initialize(bench, remorque, charge_init, tournees_data)
        @b=bench

        # Etat de validité de la solution (necessite appel à update)
        @valid = true

        # texte qui contiendra une erreur par ligne
        @errmsg = ""

        # la remorque (i.e. remorque) concerné par ce circuit
        @remorque = remorque

        # Chargement initial de la remorque
        @charge_init = charge_init

        # liste des station visités par ce circuit (i.e. ce remorque)
        @stations = Array.new()

        # depots : hash indicée par les stations
        ## @depots = depots ? depots.dup : Hash.new
        @depots = Hash.new

        for station, depot in tournees_data
            @stations << station
            @depots[station] = depot
        end

        # charge de la remorque **après** passage à une station
        # (utilisé pour vérifier la remorque)
        @charges = Hash.new
        ## @charges_min = Hash.new
        ## @charges_max = Hash.new

        # Nombre de visites à la station (devra être exactemenet 1)
        # La closure permet de donner une valeur par défaut de zéro, ce qui
        # permet d'incrémenter sans initialiser
        @nb_visits = Hash.new{|key, val| 0} # Fonctionne aussi
        # @nb_visits = Hash.new(0) # Fonctionne

        # Déficit par station (relatif !)
        @deficits = Hash.new

        # Désequilibre total (toujours positif ou nul !)
        @desequilibre = 0

        # longueur de ce circuit
        @length = 0

        # la liste des arcs parcourus par la remorque
        @arcs = Array.new

        # self.partial_clear
        self.update

    end
    # vide la liste des stations
    def clear
        partial_clear
        @stations.clear
        @depots.clear
        @charge_init = 0
    end
    # Ne vide que les attributs dérivés à partir de la liste des stations
    # desservies (en vue de regénérer des attributs dérivés valides)
    def partial_clear
        @charges.clear
        ## @charges_min.clear
        ## @charges_max.clear

        @nb_visits.clear
        @deficits.clear
        @desequilibre = 0
        @arcs.clear

        @errmsg = ""
        @length = 0

        # pourra être faux si la capacité de la remorque est dépassée ou si
        # arc impossible (e.g. filtrage a priori car capa remorque insuffisante)
        @valid = true
    end
    # Copie du circuit_source dans self, s'il n'est pas vide, il est
    # préalablement reinitialisé
    #
    # HYP : un circuit ne change jamais de bench ni de remorque !
    #
    # ATTENTION : JAMAIS TESTÉ !!
    #
    def copy circuit_src
        if circuit_src.b != self.b
            raise "Erreur : le circuit à copier doit avoir le même attribut bench"
        end
        if circuit_src.rem != self.rem
            raise "Erreur : le circuit à copier doit avoir le même attribut remorque"
        end
        self.clear
        @stations.concat circuit_src.stations # c'est un Array
        @depots.merge circuit_src.depots      # c'est une Hash
        update
    end

    # Mise à jour des "attributs dérivés"
    #  @valid, @errmsg, @arcs, @charges, deficits, @desequilibre, @length, ...
    # Ces attributs ne seront significatifs que si @valid vaut true
    # Si !@valid, @errmsg contient une information textuelle sur la raison
    # de la non validité de cette solution
    #
    def update
        $log.debug "Circuit#update: START @stations (size=#{@stations.size}) "
        # $log.debug "Circuit#update: START @stations (size=#{@stations.size}) " +
        #            " [" + stations.join(",") + "]"
        partial_clear
        ## puts "Circuit#update: apres partial_clear @nb_visits:#{@nb_visits}"
        # On construit la liste des arcs.
        # Un circuit nul contient au moins l'arc remorque->remorque de longueur nulle
        #
        ([remorque] + @stations + [remorque]).each_cons(2) do | src, dst |
            arc = @b.get_arc(src, dst)
            if !arc
                @errmsg = "Arc inexistant entre #{src} et #{dst}"
                @valid=false
                # raise "\n" + @errmsg + "\n"
                # STDERR.puts "\n" + @errmsg + "\n"
                $log.error  @errmsg
                next
            end
            @arcs << arc
        end

        for arc in @arcs
            $log.debug  "Circuit#update: #{remorque.name} arc: #{arc.to_s}"
        end

        # Construction des attributs
        # - @depots est connu
        # - @charge_init est connu
        #
        @charges[remorque] = @charge_init

        # Mise à jour de @length et de @charges[Station] avec test sur capa_remorque
        for arc in @arcs
            @length += arc.dist
            $log.debug "Circuit#update: arc=#{arc}=dist=#{arc.dist} (=>#{@length})"

            # Mise à jour de @charge sauf pour l'arc de retour final
            break if arc.dst.is_a? Remorque
            new_charge = @charges[arc.src] - @depots[arc.dst]
            ## new_charge_max = [@charges_max[arc.src] - @depots[arc.dst] , @b.capa_remorque].man
            ## new_charge_min = [@charges_min[arc.src] + @depots[arc.dst] , @b.capa_remorque].min
            if ! new_charge.between?(0, @remorque.capa)
                @errmsg << "arc #{arc.name} non faisable : capa dépassée " +
                           "new_charge=#{new_charge}\n"
                @valid = false
            end
            @charges[arc.dst] = new_charge
        end #for arc
        # Mise à jour des deficits pour ce circuit
        for st in @stations
            # TODO : vérifier ici qu'on ne sort pas des bornes de la station
            new_nbvp = st.nbvp + @depots[st]
            if ! new_nbvp.between?(0, st.capa)
                @errmsg << "Station #{st.name} hors capacité [0, #{st.capa}] : " <<
                           "new_nbvp=#{new_nbvp}\n"
                @valid = false
            end

            deficits[st] = (st.nbvp + @depots[st]) - st.ideal
        end
        # Mise à jour du déséquilibres total de ce circuit
        @desequilibre = @stations.reduce(0){ |tot, st| tot += deficits[st].abs }

        # Mise à jour du nombre de visites par station : c'est le nombre d'occurences
        # d'une station dans la liste @stations
        for st in @stations
            ## puts "Incrément de 1 pour st#{st.name} valait #{@nb_visits[st]}"
            @nb_visits[st] += 1
        end
        # # Chaque station **de ce circuit** doit être servie une et une seule fois
        # #
        # # @stations.each{|st| puts "==-  nb_visits pur {@rem.name}: #{st.name} => #{@nb_visits[st]}"}
        # for st in @b.stations
        #     ## puts station.to_s_full
        #     nb = @nb_visits[st]
        #     if nb != 1
        #         @errmsg << "!Erreur : station #{st.name} desservie #{nb} fois !\n"
        #         @valid = false
        #     end
        # end

        # ASSERT : les attributs dérivés de ce circuit sont à jour
        $log.debug  "Circuit#update END => FAISABLE=" +  (@valid ? "true" : "false")
    end
    # calcul de la longueur totale du circuit fermé depuis le tic en passant
    # par toutes les interventions puis retour au tic
    #
    # HYPOTHESE : le circuit est valide.
    #
    def length
        return @length
    end

    # retourne une description détaillée du circuit
    def to_s
        txt = "# Circuit associé à la remorque #{remorque.name} "
        txt << "de capa #{@remorque.capa} " << "\n"
        txt << "#       id, charge_init, desequ, longueur\n"
        txt << "circuit #{remorque.name} #{@charge_init} #{@desequilibre}  #{length}\n"

        for station in @stations
            txt << "#{station.name.to_s.rjust(4)} #{@depots[station].to_s.rjust(4)}\n"
        end
        txt << "\n"
    end
    # les informations détaillées du trajet pour ce circuit
    def to_s_detail
        txt = "Détail de la tournée de la remorque #{@remorque.name} "
        txt << "de capacité #{@remorque.capa}\n"
        txt << "  charge_init=#{@charge_init} "
        txt << "desequilibre=#{@desequilibre}  longueur=#{length}, "
        txt << "visite de #{@stations.size} stations.\n"

        for station in @stations
            txt << "" <<
                   "   #{station.name.to_s.rjust(3)}" <<
                   " (capa: #{station.capa.to_s.rjust(2)}" <<
                   " nbvp: #{station.nbvp.to_s.rjust(2)}/#{station.ideal.to_s.ljust(2)})" <<
                   " => dépot #{@depots[station].to_s.rjust(3)},"  <<
                   "  nb_visits: #{@nb_visits[station].to_s.rjust(1)}, " <<
                   "  deficit: #{@deficits[station].to_s.rjust(2)}."    <<
                   "  charge: #{@charges[station].to_s.rjust(2)}\n"
        end
        txt << "  liste des arcs pour la tournée de #{@remorque.name} :\n"
        for arc in @arcs
            txt << "    #{arc} dist=#{arc.dist}\n"
        end
        return txt
    end
end #class Circuit

# Mémorise **une** solution de l'instance passée au constructeur
#
# Le principal attribut d'un objet Solution est une hash de circuits qui
# contient les circuits chacun associé à une remorque
#
# Les attributs file_total_length et file_total_desequilibre sont les valeurs
# prétendu dans le fichier lu (qui peuvent être différentes (donc fausses)
# des valeurs réel calculées par la validation.
#
# Voir la méthode Solution#test pour un exemple d'utilisation.
#
class Solution

    attr_accessor :circuits
    attr_accessor :desequilibre, :length
    attr_accessor :file_total_length, :file_total_desequilibre, :nb_visits
    attr_accessor :strict_sol_format
    attr_reader   :b, :valid, :errmsg

    @strict_sol_format=true

    def initialize(bench, args={})
        @strict_sol_format=args[:strict_sol_format] if args && args[:strict_sol_format]
        @b = bench

        @valid=true
        @errmsg=""

        # Hash des circuits composant la solution (un par remorque)
        @circuits = Hash.new
        # xxxx todo ajout 12/11/2014 (PSEUDO-LANGAGE)
        # for rem in bench.remorques
        #     charge_init = 0
        #     circuit_data = Hash.new
        #     @circuits[rem] = Circuit.new(@b, rem, charge_init, circuit_data)
        # end

        # Nombre de vistes pour chaque station dans une solution complète
        # (devra être exactemenet 1).
        # Fait appel à l'attribut nb_visits de chaque circuits.
        #
        @nb_visits = Hash.new{|key, val| 0}

        # Désequilibre total
        @desequilibre = 0

        # Longueur totale de la solution
        @length = 0

        # self.clear
        # raise "A FAIRE"
    end

    def clear
        partial_clear
        @circuits.each_pair do |remorque, circuit|
            # on vide le circuit de chaque tic
            circuit.clear
        end
        @circuits.clear
    end

    def partial_clear
        @circuits.each_pair do |remorque, circuit|
            # on vide le circuit de chaque tic
            circuit.partial_clear
        end
        @valid=true
        @errmsg=""
        @nb_visits.clear
        @desequilibre = 0
        @length = 0
    end

    # Assure le calcul de mise à jour des circuits individuels
    # et met à jour ses propres attributs @valid
    #
    # TODO : Réorganiser en séparant la mise à jour de @valid et de @errmsg
    #        et factoriser avec validate
    #
    def update
        $log.debug "Solution#update: START"

        # reset des attributs dérivés liés à cette solution/
        self.partial_clear

        # mise à jour des circuits (par update)
        @circuits.each_pair do | remorque, circuit|
            $log.debug "Solution#update: remorque===#{remorque.name}"
            circuit.update

            if !circuit.valid
                @valid=false
                # HYP il y a déjà un "\n" final dans circuit.errmsg
                @errmsg << circuit.errmsg
                $log.warn "Solution#update: NON FAISABLE"
                # return
            end
            $log.debug "Solution#update: FAISABLE"
        end

        # Mise à jour du nombre de visites pour chaque station
        # du désequilibre total t de la longueur totale
        #
        # HYPOTHESE : RAZ faite !
        @circuits.each_pair do |remorque, circuit|
            @desequilibre += circuit.desequilibre
            @length += circuit.length
            # Attention de ne pas itérer sur la liste des stations qui peuvent
            # être dupliquées (sinon on va compter deux fois chaque duplication
            # car cette duplication éventuelle est déjà prise en compte par
            # l'objet circuit lui-même !)
            for st in circuit.stations.uniq
                @nb_visits[st] += circuit.nb_visits[st]
            end
        end

        # Chaque station doit être servie une et une seule fois
        #
        # @b.stations.each{|st| puts "=== nb_visits: #{st.name} => #{@nb_visits[st]}"}
        for station in @b.stations
            ## puts station.to_s_full
            nb = @nb_visits[station]
            if nb != 1
                @errmsg << "!Erreur : station #{station.name} desservie #{nb} fois !\n"
                @valid = false
            end
        end
        # puts "FIN"; exit
        #

        if $log.debug?
          puts "YYYYY Solution#update: AVANT affichage de  @nb_visits.to_s "
          # puts @nb_visits.size      # ok => 500 pour instance v9 de p2016
          # puts @nb_visits.class     # ok => Hash
          # puts type_of @nb_visits   # plante
          # ATTENTION : CETTE LIGNE PLANTE LE CODE !!?? (suspension !)
          # $log.debug "Solution#update: FIN @nb_visits=#{@nb_visits.inspect}"
          for station in @b.stations
              puts "YYYYY  @nb_visits[#{station}]= #{ @nb_visits[station]}"
          end
          # # puts @nb_visits.inspect
          # # puts @nb_visits.to_s   # plante
          # # puts type_of @nb_visits   # plante
          # # puts @nb_visits.size  # ok
          puts "YYYYY Solution#update: APRES affichage de  @nb_visits.to_s "
        end
    end

    def read(path)
        if path && File.exist?(path) && File.readable?(path)
            self.clear
            read_velib_sol(path)
        else
            puts path + " : Impossible de lire le fichier " + path
            exit
        end
    end

    # read_velib_sol : lit un fichier solution et construit les attributs de self.
    #
    # Exemple de fichier solution à lire
    #
    #    nom instance_1
    #    desequilibre 0
    #    distance 306
    #
    #    circuit r1 4 0 151
    #    s5   0
    #    s2   4  # commentaire en fin de ligne
    #    s3  -3
    #    s1   0
    #    end
    #
    #    circuit r2 4 0 155
    #    s4   2
    #    s6  -1
    #    s7   0
    #    s8  -1
    #    end
    #
    def read_velib_sol(path)

        $log.debug "Solution#read_velib_sol: START path=#{path}) "

        # Quelques variables locales à cette méthode

        # Les performances prétendues dans le fichier solution. Elle devront être
        # comparées aux valeurs réelles calculées par la méthode validate_solution
        claimed = OpenStruct.new
        # Les performances globales de la solutions
        claimed.desequilibre  =  nil
        claimed.length = nil
        # Les performances de chaque remorque
        claimed.desequilibres = Hash.new
        claimed.lengths = Hash.new
        claimed.charge_inits = Hash.new # ajout diam 03/12/2015
        for rem in @b.remorques
          claimed.desequilibres[rem.name] = nil
          claimed.lengths[rem.name] = nil
          claimed.charge_inits[rem.name] = nil # ajout diam 03/12/2015
        end

        # tournees_data contiendra les données permettant de construire un Circuit
        # Les éléments seront de type [Circuit, depot:int]
        # Quelques éléments : :name, :desequilibre, :length, :stations_depots.
        #
        circuit_data = Hash.new

        states = [:in_header, :in_circuit, :end]
        state = :in_header

        $log.debug "Lecture du fichier '#{path}'"

        lines = IO.readlines(path)
        lines.each_index do |line_idx|
            line = lines[line_idx]
            $log.debug("Debut trait. de la line__#{line_idx}='#{line.chomp}'")

            # On vire les commentaires et espaces parasites
            line = line.gsub(/#.*$/, "").strip

            # On ignore si lignes vides
            next if  line.empty?

            case state
            when :in_header
                key, data = line.split(/\s+/, 2)
                case key
                when "nom"
                    @the_name = data
                    $log.debug "lect @the_name=#{@the_name}"
                    next
                when "desequilibre"
                    claimed.desequilibre = data.to_f.round
                    $log.debug "lect desequilibre=#{claimed.desequilibre}"
                    next
                when "distance"
                    claimed.length = data.to_f.round
                    $log.debug "lect length=#{claimed.length}"
                    next
                when /^tournee|circuit$/
                    state = :in_circuit
                    $log.debug "lect passage dans le state #{state}"
                    circuit_data.clear

                    name, charge_init, desequ, len = data.split /\s+/
                    remorque = b.find(name)
                    if !remorque
                        STDERR.puts "ERREUR : #{name} n'est pas un nom de remorque connu !"
                        exit 1
                    end
                    circuit_data[:desequilibre] = desequ.to_f.round
                    circuit_data[:length] = len.to_f.round
                    circuit_data[:remorque] = remorque
                    circuit_data[:charge_init] = charge_init.to_f.round
                    circuit_data[:stations_depots] = Array.new
                    next
                else
                    # On pourrait choisir d'ignorer un mot-clé inconnu
                    raise "Erreur : en-tête inconnu #{key} dans #{line}"
                end
            when :in_circuit
                if line.downcase == "end"
                    # la tournee de la remorque est terminée : on va construire le
                    # circuit associé à partir des données lues

                    # On récupère l'objet remorque
                    remorque = circuit_data[:remorque]
                    claimed.desequilibres[remorque.name] = circuit_data[:desequilibre]
                    claimed.lengths[remorque.name] = circuit_data[:length]
                    claimed.charge_inits[remorque.name] = circuit_data[:charge_init] # ajout 03/12/2015

                    circuit = Circuit.new(@b, circuit_data[:remorque],
                                              circuit_data[:charge_init],
                                              circuit_data[:stations_depots] )

                    # On mémorise ce circuit
                    @circuits[circuit_data[:remorque]] = circuit

                    state = :in_header
                    $log.debug "lect passage dans le state #{state}"

                    ### break
                else
                    # On doit lire les info de la station visitée (nom et dépot)
                    # par la tournée en cours
                    name, depot = line.split /\s+/
                    depot = depot.to_f.round
                    station = @b.find(name)
                    if !station
                        STDERR.puts "ERREUR : #{name} n'est pas un nom de station connu !"
                        exit 1
                    end
                    circuit_data[:stations_depots] << [station, depot]

                end

            when :end
                break # On ne devrait pas arriver là !
            else
                raise "Erreur : état #{state} inconnu au lieu de #{states.inspect}"
            end
        end # lines.each

        # On vérifie que toutes les remorques apparaissent dans le fichier
        all_remorques_found = true
        for rem in @b.remorques
            if ! @circuits.has_key?(rem)
                all_remorques_found = false
                print "manque circuit pour la remorque #{rem.name}\n"
            end
        end
        if not all_remorques_found
            STDERR.puts "Fichier solution incomplet : on arrête."
            exit(1)
        end
        $log.debug "claimed=#{claimed.inspect}"
        claimed_hash = claimed.to_h

        $log.debug "Avant mise à jour des attributs dérivés de la sol (dont arcs, ...)"
        self.update
        $log.debug "Après mise à jour des attributs dérivés"

        # Quelques tests de cohérence supplémentaires de l'instance
        self.validate_solution(claimed)
        $log.debug "Solution#read_velib_sol END : @valid=#{@valid}"
    end

    # Méthode de haut niveau succeptible d'afficher des messages et
    # d'interrompre le programme.
    #
    # Cette méthode vérifie deux types d'erreurs :
    # - les viols de contraintes compte tenue de la solution (ordre de visites des
    #   stations et valeurs des dépots de vélos)
    # - le désaccord entre les valeurs prétendues (dans le fichier solution) et
    #   les valeurs déduites des informations de dépots.
    #
    # Entré :
    # - claimed : OpenStruc contenant les valeurs prétendues par le fichier solution
    #   (ou espérées)
    #   Ces valeurs seront comparées au valeurs exactes déduites de l'ordre de
    #   visite et des dépots en chaque station.
    #
    # Sortie :
    # - positionne l'attribut booléen @valid et renseigne éventuellement la chaine
    #   @errmsg
    # - retourne l'attribut @valid même si redondant
    # - EFFET DE BORD : arete le programme en cas d'erreur sauf si l'attribut
    #   @strict_sol_format est positionné à faux.
    #
    def validate_solution(claimed)

        $log.debug "Solution#validate_solution: START (reçoit claimed)"

        # ATTENTION: ici on ignore le tavail de vérif fait par l'appel précédent
        # à Solution#update
        @valid = true
        @errmsg = ""

        # 2 - vérif. que chaque circuit est faisable au niveau de la remorque
        #     (compte tenu des dépots de vélos et de sa capacité)
        #
        for rem in @b.remorques
            $log.debug "validate_solution pour rem ${rem.name}"
            circuit = @circuits[rem]
            # On vérifie que chaque circuit est individuellement faisable
            if ! circuit.valid
                @errmsg << "!Erreur : circuit pour #{rem.name} non \"faisable\" :\n"
                circuit.errmsg.strip.split(/\n"/).each do | line |
                    @errmsg << "   #{line}\n"
                end
                @valid = false
            end
            # même si le circuit est valid, il faut s'assurer que les indications
            # du fichier (desequ et length) pour cette tournée sont correctes
            if circuit.length != claimed.lengths[rem.name]
                @errmsg << "!Erreur : longueur circuit pour #{rem.name} prétendue: " <<
                           "#{claimed.lengths[rem.name]} réelle: #{circuit.length}\n"
                @valid = false
            end
            if circuit.desequilibre != claimed.desequilibres[rem.name]
                @errmsg << "!Erreur : desequilibre circuit pour #{rem.name} prétendu: " <<
                           "#{claimed.desequilibres[rem.name]} réel: #{circuit.desequilibre}\n"
                @valid = false
            end
            # Ajour diam le 03/12/2015
            if !claimed.charge_inits[rem.name].between?(0, rem.capa)
                @errmsg << "!Erreur : charge initiale pour #{rem.name} prétendue: " <<
                           "#{claimed.charge_inits[rem.name]} mais hors capacité : [0, #{rem.capa}]\n"
                @valid = false
            end
        end

        # 3 - Chaque station existante doit être servie une et une seule fois
        #
        ### pp @nb_visits
        for station in @b.stations
            $log.debug "validate_solution pour station ${station.name}"
            ### puts station.to_s_full
            nb = @nb_visits[station]
            if nb != 1
                @errmsg << "!Erreur : station #{station.name} desservie #{nb} fois !\n"
                @valid = false
            end
        end

        # - la longueur de chaque circuit doit être celle spécifiée dans le fichier
        #
        # En cas où le solution est fausse : self.length peut ne pas être calculable
        the_length = self.length || 0
        if claimed.length != the_length
            @errmsg << "!Erreur : longueur totale attendue: #{claimed.length}, " <<
                       " réelle: #{the_length}\n"
            @valid = false
        end
        $log.debug "LONGUEUR REELE DE LA SOLUTION #{the_length}"
        $log.debug "LONGUEUR PRETENDUE DE LA SOLUTION #{claimed.length}"

        # - Vérification du déséquilibre prétendu par rapport au déséquilibre réél
        #
        # En cas où le solution est fausse : self.desequilibre peut ne pas être calculable
        the_desequilibre = self.desequilibre || 0
        if the_desequilibre != claimed.desequilibre
            @errmsg << "!Erreur : déséquilibre global attendu: #{claimed.desequilibre}, " <<
                       "réél: #{the_desequilibre}.\n"
            @valid = false
        end

        # Comportement final (pas clair : arret ? exception ? valeur de retour ?)
        # Le comportemenet en cas d'erreur dépend de l'option @strict_sol_format
        if @valid
            STDERR.puts "Solution valide."
        else
            STDERR.puts "Solution incorrecte :\n#{@errmsg}"
        end
        if !@valid  && @strict_sol_format
            STDERR.puts "On arrête car solution non valide et option --strict_sol_format positionnée."
            exit 1
        end
        return @valid
    end

    # Construction d'une chaine correspondant au format standard de la solution
    #
    def to_s
        # equivalent à Time#strftime("%a %b %d %H:%M:%S %Z %YÕ")
        # txt =  "# généré par programme le #{Time.now.strftime("%Y-%m-%dT%H:%M:%S%z")}\n\n"
        txt = ""

        # Dans le cas simplifié, il n'y a qu'une seule remorque, donc une seule
        # variable charge_init à afficher.
        # En réalité, cette variable virtuelle correspond à la variable charge_init
        # réelle associée au premier (et unique) remorque
        charge_init = @circuits[@b.remorques[0]].charge_init  # PAS TRÉS CLAIR !??
        txt << "nom #{@b.name}\n" <<
               "desequilibre #{desequilibre}\n" <<
               "distance #{length}\n" <<
               "\n"


        for rem_name, circuit in @circuits
            ### txt << "# Détail des dépots pour la remorque #{circuit.remorque.name}\n"
            txt << "#       id, charge_init, desequ, longueur\n"
            txt << "circuit #{rem_name} #{circuit.charge_init} "
            txt << "#{circuit.desequilibre} #{circuit.length}\n"
            for st in circuit.stations
                txt << "%4s %4d\n" % [st.name, circuit.depots[st]]
            end
            txt << "end\n\n"
        end
        return txt
    end
    def to_s_detail
        txt =  "Détail de la solution pour l'instance velib : #{@b.name}\n"

        # On doit explore **toutes** les stations de l'instance pour savoir si
        # elles sont visitées ou pas.
        bad_nb_visits = Hash.new
        for st in @b.stations
            bad_nb_visits[st] = @nb_visits[st] if @nb_visits[st] != 1
        end
        # On extrait de la Hash la partie ayant un nombre incorrect de visites
        ## bad_nb_visits = nb_visits.select{|station, nbvis| nbvis != 1}
        if bad_nb_visits.empty?
            txt << "Toutes les stations ont été visitées exactement 1 fois.\n"
        else
            txt << "Liste des stations dont le nombre de visites est différent de 1 :\n"
            for station, nbvis in bad_nb_visits.sort
                txt << "  #{station.name} est visitée #{nbvis} fois !!\n"
            end
        end

        txt << "Cumul des longueurs :    #{length.to_s.rjust(5)}\n"
        txt << "Cumul de désequilibres : #{@desequilibre.to_s.rjust(5)}\n"

        @circuits.each do | rem_name, circ |
            txt << circ.to_s_detail
        end

        return txt
    end


    # Utilisé par Solution#test
    def Solution.new_velib_mini_sol(b)

        # Extraction des objets station de l'instance
        ## s1, s2, s3, s4, s5 = [b.find("1"),b.find("2"),b.find("3"),b.find("4"),b.find("5")]
        s = []
        for i in 1..8
            s[i] = b.find("s#{i}")
        end

        # Construction des éléments de la tournée liée à la remorque r1
        r1 = b.find("r1")
        charge_init_1 = 4
        tournees_data_1 = [
            [s[5], 0],
            [s[2], 4],
            [s[3],-3],
            [s[1], 0]
        ]
        circuit_1 = Circuit.new(b, r1, charge_init_1, tournees_data_1)

        # Construction des éléments de la tournée liée à la remorque r1
        r2 = b.find("r2")
        charge_init_2 = 4
        tournees_data_2 = [
            [s[4], 2],
            [s[6],-1],
            [s[7], 0],
            [s[8],-1]
        ]
        circuit_2 = Circuit.new(b, r2, charge_init_2, tournees_data_2)


        # Construction de la solution
        sol = Solution.new b
        sol.circuits[r1] = circuit_1
        sol.circuits[r2] = circuit_2
        sol.update
        return sol
    end
    def Solution.new_velib_mini_bad_sol(b)

        # Extraction des objets station de l'instance
        ## s1, s2, s3, s4, s5 = [b.find("1"),b.find("2"),b.find("3"),b.find("4"),b.find("5")]
        s = []
        for i in 1..8
            s[i] = b.find("s#{i}")
        end

        # Construction des éléments de la tournée liée à la remorque r1
        r1 = b.find("r1")
        charge_init_1 = 4
        tournees_data_1 = [
            # [s[5], 0],
            [s[5], 3],
            # [s[2], 4],
            [s[2], 1],
            # [s[3],-3], # XXXX suppression de la visite de s{3}
            # [s[1], 0]  # XXXX modif dépot de 0 à 10 pour s{1}
            [s[1], -3]
        ]
        circuit_1 = Circuit.new(b, r1, charge_init_1, tournees_data_1)

        # Construction des éléments de la tournée liée à la remorque r1
        r2 = b.find("r2")
        charge_init_2 = 4
        tournees_data_2 = [
            [s[4], 2],
            [s[1], 1], # XXXX visite supplémentaire
            [s[6],-1],
            [s[7], 0],
            [s[1], 1], # XXXX visite supplémentaire
            [s[8],-1]
        ]
        circuit_2 = Circuit.new(b, r2, charge_init_2, tournees_data_2)


        # Construction de la solution
        sol = Solution.new b
        sol.circuits[r1] = circuit_1
        sol.circuits[r2] = circuit_2
        sol.update
        return sol
    end



    # test1 : test d'une solution correcte pour l'instance mini
    #
    def Solution.test1()
        puts "="*74 + "\n== TEST 1 : instance velib_1 et solution correcte\n\n"
        b = Bench.new_velib_mini
        puts "="*74 + "\n== Affichage de b.to_s\n\n"
        puts b.to_s

        # Construction de la solution
        sol = Solution.new_velib_mini_sol(b)

        puts "="*74 + "\n== Affichage par sol.to_s\n\n"
        puts sol.to_s

        puts "="*74 + "\n== Affichage par sol.to_s_detail\n\n"
        puts sol.to_s_detail

        puts "="*74 + "\n== Validation de sol\n\n"
        claimed = OpenStruct.new
        claimed.length = 306
        claimed.desequilibre = 0

        r1 = b.find "r1"
        r2 = b.find "r2"

        claimed.lengths = {
            r1 => 151,
            r2 => 155
        }
        claimed.desequilibres = {
            r1 => 0,
            r2 => 0
        }
        sol.strict_sol_format = false # pour éviter l'exit interne
        sol.validate_solution(claimed)
        puts "Résultat de la validation : sol.valid=#{sol.valid}"
        puts "avec le message d'erreur de longueur (0?): #{sol.errmsg.length}"
        puts "="*74 + "\n== TEST 1 fini\n" + "="*74
    end

    # test2 : test d'une solution fausse pour l'instance mini
    #
    def Solution.test2()
        puts "="*74 + "\n== TEST 2 : instance velib_1 et solution fausse\n\n"
        b = Bench.new_velib_mini
        puts "="*74 + "\n== Affichage de b.to_s\n\n"
        puts b.to_s

        # Construction de la solution
        sol = Solution.new_velib_mini_bad_sol(b)

        puts "="*74 + "\n== Affichage par sol.to_s\n\n"
        puts sol.to_s

        puts "="*74 + "\n== Affichage de sol apres update\n"
        puts sol.to_s_detail

        puts "="*74 + "\n== Validation de sol (doit être incorrect)\n\n"
        claimed = OpenStruct.new
        claimed.length = 1306
        claimed.desequilibre = 10

        r1 = b.find "r1"
        r2 = b.find "r2"

        claimed.lengths = {
            r1 => 151,
            r2 => 155
        }
        claimed.desequilibres = {
            r1 => 0,
            r2 => 0
        }

        sol.strict_sol_format = false # pour éviter l'exit interne
        sol.validate_solution(claimed)
        puts "Résultat de la validation : sol.valid=#{sol.valid}"
        puts "avec le message d'erreur de longueur (524 attendu ?): #{sol.errmsg.length}"
        puts "="*74 + "\n== TEST 2 fini\n" + "="*74
    end

    def Solution.test()
        Solution.test1
        Solution.test2
    end

end #class Solution

def main(argv)

    # 1 - lecture des arguments avec initialisation des options de l'appli
    date1 = Time.now

    opts = Args.new(argv)
    $log.info "Arguments lus."
    # puts "Arguments lus."

    # Si on est en mode génération d'instance
    if opts.generate
        b = Bench.generate opts
        $log.info "Instance générée : #{b.name}."

        # Le reste de ce bloc consiste à savoir ce qu'on doit faire de línstance
        #

        ### puts b.to_s
        ### exit
        # puts "opts.datfile=#{opts.datfile}"
        if !opts.datfile || opts.datfile == '-'
            $log.debug "main : dans test gen et .dat inexistant ou stdout demandée"
            # On  utilise la sortie standard
            puts b.to_s
            # Si on utilise la sortie standard, on doit arreter ici !
            $log.warn "Aucun nom de fichier d'instance précisé pour la sauvegarde."
            $log.warn "On a fini (vous pouvez préciser -d AUTO comme nom de fichier)."
            exit 0
        end

        if opts.datfile == "AUTO"
            # On doit générer un nom du fichier à partir du non de l'instance
            opts.datfile = "#{b.name}.dat"
        end

        # assert : opts.datfile est défini

        if File.exists?(opts.datfile) && !opts.force
            $log.error "Fichier #{opts.datfile} existe déjà : on arrète (voir --force)"
            exit 1
        elsif File.exists?(opts.datfile) && opts.force
            $log.warn "Ecrasement du fichier existant : #{opts.datfile}"
        else
            # assert  !File.exists?(opts.datfile)
            $log.warn "Création du fichier : #{opts.datfile}"
        end

        # On écrit le fichier généré (qu'il préxiste ou non)
        File.open( opts.datfile, "w") do |fid|
            fid.write b.to_s
        end
    end

    # 2 - Construction de l'instance
    if opts.datfile
        b = Bench.new(opts.datfile)
        $log.info "Lecture instance #{opts.datfile} faite (#{b.name})."

        if opts.show_in_file
            $log.info "Traitement de show_in_file"
            # puts "#" + "="*72 + "\n# Regénération de l'instance lue :\n\n"
            puts b.to_s
            puts
        end

        if opts.show_in_detail
            $log.info "Traitement de show_in_detail"
            puts b.to_s_detail
            puts
        end

        if opts.show_in_opl
            $log.info "Traitement de show_in_opl"
            # puts "// " + "="*72 + "\n// Regénération dans le format de données \"OPL\":\n\n"
            puts b.to_s("opl")
            puts
        end

        if opts.show_in_dists
            $log.info "Traitement de show_in_dists"
            puts "#"+"="*72 + "\n# Affichage des distances inter sites :\n\n"
            puts b.dists_to_s
            puts
        end

    else
        puts "ERREUR : pas de fichier d'instance à lire (-h pour l'aide)"
        exit
    end

    # 2 - Lecture du fichier solution
    if opts.solfile
        sol = Solution.new(b, :strict_sol_format => opts.strict_sol_format)
        sol.read(opts.solfile)
        $log.info "Lecture fichier solution faite : #{opts.solfile}"
        if opts.show_sol_file
            puts "# "+"="*70 + "\n# Affichage de la solution regénérée :\n\n"
            puts sol.to_s
            puts
        end
        if opts.show_sol_detail
            puts "\n"+"="*72 + "\nAffichage des détails de la solution :\n\n"
            puts sol.to_s_detail
            puts
        end
    end

    # Affichage du pdf (avec ou sans une solution)
    if opts.show_sol_pdf
        $log.info "Traitement de show_sol_pdf (instance avec ou sans solution)"

        # Pour l'instant on fige en dur le nom du fichier pdf généré.
        # pdfname = b.name + ".pdf"
        pdfname = opts.pdffile
        okstatus = b.write_pdf(pdfname, sol)
        if okstatus
            puts "Fichier #{pdfname} créé avec succès\n\n"
        else
            puts "Erreur lors de la création du fichier #{pdfname}\n\n"
        end
        # puts
    end


    # 3 - Exploitation de l'instance
    # TODO Afficher des statistiques, ...
    date2 = Time.now
    # puts "# Durée d'exécution : " + (date2-date1).to_s + "s"
    $log.info "# Durée d'exécution : " + (date2-date1).to_s + "s"
end

# L'application principale consiste en ce fichier (i.e. si ce fichier n'est
# pas utilisé que comme bibliothèque de classe) alors on doit lancer la fonction main
# Si ce fichier est utilisé comme bibliotheque de classe et que l'application réellement
main ARGV if __FILE__ == $0

#./
