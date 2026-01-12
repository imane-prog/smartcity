🏙️ Smart City Simulation - Raylib
Ce projet est une plateforme de simulation urbaine interactive conçue pour modéliser le comportement des véhicules autonomes et la gestion intelligente des infrastructures de stationnement dans une ville connectée.

👥 Équipe de Développement
Ce projet est le fruit du travail collaboratif de :

Imane Rhanebou

Mohamed Zarkik

Abdallaoui Alaoui Mohamed

🚀 Fonctionnalités Clés
Gestion de Trafic Avancée : Simulation de 50 véhicules avec détection de files, maintien des distances de sécurité (SAFE_DISTANCE) et gestion des changements de voie fluides.

Écosystème de Parkings : Quatre zones de stationnement distinctes (VIP, Central, Eco, City) dotées de capacités et de tarifications variables, allant de 2.00 dh/h à 15.00 dh/h.

Interface Utilisateur (UI) Dynamique :

Dashboard : Suivi en temps réel de l'occupation (spotsOccupied) via des barres de progression colorées (Vert/Orange/Rouge).

Contrôle Audio : Interface complète pour la gestion du volume et du mode muet de la musique d'ambiance.

Chronomètre : Mesure précise de la durée de la simulation.

🛠️ Concepts Techniques de Programmation
Le logiciel a été développé en C++ en mettant l'accent sur la robustesse et l'efficacité.

1. Gestion de Mémoire et POO
Constructeurs par Listes d'Initialisation : Optimisation des performances lors de la création des objets Car et ParkingLot pour garantir un état initial propre.

Gestion des Ressources (RAII) : Utilisation de destructeurs pour assurer le déchargement systématique des textures (UnloadTexture) et des flux audio (UnloadMusicStream) à la fermeture du programme.
2. Design Patterns
Game Loop : Une boucle principale gérant l'actualisation (Update) et l'affichage (Draw) à 60 FPS.

State Pattern : Une machine à états gère la navigation fluide entre l'introduction, la fiche d'information et la simulation.
3. Algorithmique et Logique
Conversion de Types : Application des règles de promotion intégrale du C++ lors des opérations entre int et char pour la gestion des index de places.

Géométrie de Simulation : Utilisation de Vector2Normalize et Vector2Distance pour calculer les trajectoires des véhicules et les orientations par rapport au centre de la ville.

⚙️ Configuration et Installation
Prérequis
Bibliothèque Raylib installée sur votre système.

Compilateur C++ compatible (GCC recommandé).

4. Compilation
Exécutez la commande suivante pour compiler le projet :
cmake --build build 

.\build\TrafficTests.exe    (tests)                                                                                     

.\build\SmartCitySim  .exe


Bash
g++ -o SmartCity main.cpp -lraylib -lopengl32 -lgdi32 -lwinmm

Structure des Ressources
assets/accueil.png : Image d'introduction.

assets/menu.mp3 : Musique de fond.

🎮 Mode d'Emploi
Lancement : L'écran d'accueil présente les auteurs. Appuyez sur ESPACE.

Configuration : Consultez la fiche de tarifs. Réglez le volume audio si nécessaire, puis cliquez sur Démarrer.

Observation : Les véhicules (state = DRIVING) circulent et cherchent des places libres (firstFreeSpot). Lorsqu'ils ont terminé, ils passent à l'état PARKED.