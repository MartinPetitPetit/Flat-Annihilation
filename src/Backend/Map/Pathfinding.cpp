/*
 * Backend/Map/Pathfinding.cpp
 *
 * Rôle du fichier :
 * Keeps a legacy empty pathfinding file to avoid duplicate symbols when older map pathing files still exist.
 *
 * Notes de lecture :
 * Ce fichier appartient au module Map. Il participe à la génération procédurale, à la structure de la carte ou aux anciens points de compatibilité.
 * Les commentaires ajoutés servent uniquement à expliquer le code.
 * La logique originale du programme n'a pas été modifiée.
 */

/*
 * Ancien emplacement du pathfinding.
 *
 * Les fonctions findPath(...) et formationDestinations(...)
 * sont maintenant compilées depuis :
 *   src/Backend/Pathing/Pathfinding.cpp
 *
 * Ce fichier reste volontairement sans définition pour éviter
 * les doubles symboles au linkage si un ancien fichier existe encore
 * dans src/Backend/Map/.
 */
