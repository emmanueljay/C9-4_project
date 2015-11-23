

# Cette commande non obligatoire est fortement conseillée.
cmake_minimum_required (VERSION 2.6)

# Déclaration du projet.
# Les fichiers CMakeLists pourront référer à la racine du projet par la variable
# ${VELIB_SOURCE_DIR} et ${VELIB_BINARY_DIR}
project(VELIB)
set(EXE run)

# L'exécutable sera rangé différemment en fonction de la plateformee.
# Par défaut le binaire est construit dans le répertoire courant (?)
# set(EXECUTABLE_OUTPUT_PATH ../bin)
# set( EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE} )

#     SET (CMAKE_C_COMPILER             "/usr/bin/clang")
#     SET (CMAKE_C_FLAGS                "-Wall -std=c99")
#     SET (CMAKE_C_FLAGS_DEBUG          "-g")
#     SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
#     SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
#     SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
#
#     SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
#     SET (CMAKE_CXX_FLAGS                "-Wall")
#     SET (CMAKE_CXX_FLAGS_DEBUG          "-g")
#     SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
#     SET (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
#     SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
#
#     SET (CMAKE_AR      "/usr/bin/llvm-ar")
#     SET (CMAKE_LINKER  "/usr/bin/llvm-ld")
#     SET (CMAKE_NM      "/usr/bin/llvm-nm")
#     SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
#     SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib")


# Les options de compilation
# add_definitions(-Wall -Wextra -ansi -O2 -Wwrite-strings
#                 -Wuninitialized -Wno-missing-braces
#                 -Wno-missing-field-initializers)
add_definitions(
    -Wall
    # -Wno-unused-private-field  # Pour inhiber un warning de cplex 12.4
    -m64
    -fPIC
    -fexceptions
    -DNDEBUG
    # -DIL_STD  # Pour cplex en C++
    -std=c++11
    -O
)

# Config spécifique à un système
# - UNIX vrai sous une plateforme Unix (don macosx)
# - APPLE vrai pour un os apple (donc macosx !)
# - WIN32 : vrai sous Windows (32 et 64bit !)
if(APPLE)
    message("=> Détection système Apple")
    set(CMAKE_EXE_LINKER_FLAGS  "-Wl,-no_compact_unwind")
endif()


# file(
#     GLOB
#     srcs
#     src/*.cpp
#     src/*.cc
#     src/*.c
#     src/*.hh
#     src/*.h
# )

# On définit la liste des tous les fichiers sources (dont les includes)
# cmale se débrouillera pour trouver le main() et les dépendances.
# On peut préciser des sous-répertoires, des groupes de fichiers, ...
# Mais ATTENTION aux fichiers inutilisés : ils seront compiler et peuvent
# provoquer des erreurs. Il peut donc être référable de préciser exactement les
# fichiers sources
file(
    GLOB
    srcs
    # src/site.*
    # src/station.*
    # src/remorque.*
    # src/arc.*
    # src/util.*
    # src/logger.*
    # src/options.*
    # src/bench.*
    # src/*Solver.*
    # src/solver.*
    # src/cir.*
    # src/solu.*
    # src/main.*
    src/*hpp
    src/*cpp
    src/*.cc
    src/*.hh
    src/*.c
    src/*.h
    # src/*.*
)

# Le ou les exécutables à générer
add_executable(${EXE} ${srcs})

# Les répertoire ou chercher vos includes (ou ceux de cplex, ...)
include_directories(
  # ${PROJECT_SOURCE_DIR}
  # ${PROJECT_BINARY_DIR}
  # $ENV{ILOG_CONCERT_INCLUDE}
  # $ENV{ILOG_CPLEX_INCLUDE}
  # $ENV{LEMON_INCLUDE}
  # $ENV{GLPK_INCLUDE}
)

# Ajouter ici les répertoires de vos bib. dynamiques utilisées
#
link_directories(
  # ${PROJECT_BINARY_DIR}/lemon
  # $ENV{LEMON_LIB}
  # $ENV{GLPK_LIB}
  # $ENV{ILOG_CONCERT_LIB}
  # $ENV{ILOG_CPLEX_LIB}
)

# Ajouter ici les bibliothèques dynamiques que vous utilisez
#
target_link_libraries(${EXE} m)        # lib mathématique
target_link_libraries(${EXE} pthread)  # pour programmation multithead
# target_link_libraries(${EXE} ilocplex) # spécial cplex
# target_link_libraries(${EXE} cplex)    # spécial cplex
# target_link_libraries(${EXE} concert)  # spécial cplex
# target_link_libraries(${EXE} emon)   # lib de graphe
# target_link_libraries(${EXE} glpk)   # solveur PLNE gratuit




# La liste des exécutables mono-fichiers à compiler
## set(demo_srcs
##     main.cc
##     test1.cc
##     test2.cc
## )
# file(GLOB demo_srcs src/*.cc)


## foreach(demo_src ${demo_srcs})
##     get_filename_component( demo_name ${demo_src} NAME_WE )
##     message("=> demo_src=${demo_src}")
##     message("=> demo_name=${demo_name}")
##     add_executable(${demo_name} ${demo_src})
##     target_link_libraries(${demo_name} emon)
## endforeach()

# Quelques messages précisant la configuration utilisée
#
# message("=> ILOG_CPLEX_INCLUDE     is   $ENV{ILOG_CPLEX_INCLUDE}")
# message("=> ILOG_CPLEX_LIB         is   $ENV{ILOG_CPLEX_LIB}")
# message("=> ILOG_CONCERT_INCLUDE   is   $ENV{ILOG_CONCERT_INCLUDE}")
# message("=> ILOG_CONCERT_LIB       is   $ENV{ILOG_CONCERT_LIB}")
# message("=> LEMON_INCLUDE          is   $ENV{LEMON_INCLUDE}")
# message("=> LEMON_LIB              is   $ENV{LEMON_LIB}")
# message("=> GLPK_INCLUDE           is   $ENV{GLPK_INCLUDE}")
# message("=> GLPK_LIB               is   $ENV{GLPK_LIB}")
message("=> srcs                   is   ${srcs}")
# Ceci affiche 8 sur un machine 64 bits ou 4 sur un machine 32 bit
message("=> CMAKE_SIZEOF_VOID_P    is   ${CMAKE_SIZEOF_VOID_P}")
message("\n")



#
# Complément pour ajouter quelques cibles personnalisées dans le Makefile généré
#
EXECUTE_PROCESS(
    COMMAND date +%Y%m%d-%Hh%M
    OUTPUT_VARIABLE date_stamp
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
GET_FILENAME_COMPONENT( project_dir_name ${CMAKE_SOURCE_DIR} NAME )


# On peut compléter les cible du Makefile généré
# (la comande finale de cette ciblesera ajoutée à CMakeFiles/Makefile2)
# (puis Essayer de créer un cmake tbz !!)
ADD_CUSTOM_TARGET(distclean
    COMMAND @echo Nettoyage complet des sources
    COMMAND @echo \"Répertoire courant = `pwd`\"
    COMMAND @echo "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    # COMMAND make clean
    COMMAND find ${CMAKE_CURRENT_BINARY_DIR} -name "CMakeCache.txt" | xargs rm -rf
    COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/*
)
ADD_CUSTOM_TARGET(dc
    COMMAND make distclean
)
ADD_CUSTOM_TARGET(cc
    COMMAND make distclean
)
ADD_CUSTOM_TARGET(c
    COMMAND make clean
)
# Attention : cette commande construit une cible pour le Makefile.
# Il faut protéger les double-quote si l'on veux qu'elles ne soient pas consommées
# par cmake mais passée au Makefile.
# Un seul COMMENT par cible semble-t-il
ADD_CUSTOM_TARGET(tbz
    COMMENT "Création d'une archive datée du projet"
    COMMAND @echo \" => duplication du projet en : ${project_dir_name}-${date_stamp}\"
    COMMAND cp -Rp ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}-${date_stamp}
    COMMAND rm -r ${CMAKE_SOURCE_DIR}-${date_stamp}/build/
    COMMAND mkdir -p ${CMAKE_SOURCE_DIR}-${date_stamp}/build/
    # tar -C newDir :  pour se placer dans le répertoire parent de l'archive
    COMMAND tar cjf  ${CMAKE_SOURCE_DIR}-${date_stamp}.tbz
                     -C ${CMAKE_SOURCE_DIR}/..
                     ${project_dir_name}-${date_stamp}
    COMMAND rm -r ${CMAKE_SOURCE_DIR}-${date_stamp}
    COMMAND @echo \" => Archive faite :  ${project_dir_name}-${date_stamp}.tbz\"
)
ADD_CUSTOM_TARGET(txz
    COMMENT "Création d'une archive datée du projet (TEST XZ)"
    COMMAND @echo \" => duplication du projet en : ${project_dir_name}-${date_stamp}\"
    COMMAND cp -Rp ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}-${date_stamp}
    COMMAND rm -r ${CMAKE_SOURCE_DIR}-${date_stamp}/build/
    COMMAND mkdir -p ${CMAKE_SOURCE_DIR}-${date_stamp}/build/
    # tar -C newDir :  pour se placer dans le répertoire parent de l'archive
    COMMAND tar cf  ${CMAKE_SOURCE_DIR}-${date_stamp}.tar
                     -C ${CMAKE_SOURCE_DIR}/..
                     ${project_dir_name}-${date_stamp}
    COMMAND xz  ${CMAKE_SOURCE_DIR}-${date_stamp}.tar
    COMMAND mv    ${CMAKE_SOURCE_DIR}-${date_stamp}.tar.xz ${CMAKE_SOURCE_DIR}-${date_stamp}.txz
    COMMAND rm -r ${CMAKE_SOURCE_DIR}-${date_stamp}
    COMMAND @echo \" => Archive faite :  ${project_dir_name}-${date_stamp}.tbz\"
)
# ADD_CUSTOM_TARGET(zip
#     COMMENT "Création d'une archive datée du proje (TEST ZIP°t"
#     COMMAND @echo \" => duplication du projet en : ${project_dir_name}-${date_stamp}\"
#     COMMAND cp -Rp ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}-${date_stamp}
#     COMMAND rm -r ${CMAKE_SOURCE_DIR}-${date_stamp}/build/
#     COMMAND mkdir -p ${CMAKE_SOURCE_DIR}-${date_stamp}/build/
#     COMMAND echo \"1: CMAKE_SOURCE_DIR-date_stamp==1==${CMAKE_SOURCE_DIR}-${date_stamp}\"
#     COMMAND echo \"2: project_dir_name-date_stamp==2==${project_dir_name}-${date_stamp}\"
#     # tar -C newDir :  pour se placer dans le répertoire parent de l'archive
#     # COMMAND '_pwd=`pwd`'
#     # COMMAND pushd  ${CMAKE_SOURCE_DIR}/..
#     COMMAND echo 'pushd; zip -r -v -y -o -9  ${CMAKE_SOURCE_DIR}-${date_stamp}.zip \
#                      ${project_dir_name}-${date_stamp} ; popd'
#     COMMAND sh -c "pushd; zip -r -v -y -o -9  ${CMAKE_SOURCE_DIR}-${date_stamp}.zip \
#                      ${project_dir_name}-${date_stamp} ; popd"
#     # COMMAND zip -r -v -y -o -9  ${CMAKE_SOURCE_DIR}-${date_stamp}.zip
#     #                  ${project_dir_name}-${date_stamp}
#     COMMAND 'cd $_pwd'
#     # COMMAND popd
#     COMMAND rm -r ${CMAKE_SOURCE_DIR}-${date_stamp}
#     COMMAND @echo \" => Archive faite :  ${project_dir_name}-${date_stamp}.tbz\"
# )

INCLUDE(InstallRequiredSystemLibraries)
IF(WIN32 AND NOT UNIX)
    SET(CPACK_NSIS_MODIFY_PATH ON)
ENDIF(WIN32 AND NOT UNIX)

INCLUDE(CPack)

#./

