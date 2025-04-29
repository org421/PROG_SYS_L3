# Projet de Programmation Système en C

## Description

Ce projet simule un système de communication entre des **archivistes** et des **journalistes** en utilisant différentes techniques de **programmation en C**. Le système démontre l'utilisation de mécanismes de **communication entre processus (IPC)** tels que :

- **Forks** : Utilisés pour créer de nouveaux processus pour la communication entre archivistes et journalistes.
- **Signaux** : Utilisés pour le contrôle des processus et la synchronisation.
- **IPC System V** : Implémentation de la communication entre processus.
- **Sémaphores** : Utilisés pour contrôler l'accès aux ressources partagées.
- **Segments de mémoire partagée** : Permet à plusieurs processus d'accéder à une mémoire commune.
- **Files de messages** : Mise en place du passage de messages entre processus.

Le projet se compose de plusieurs fichiers source en C responsables de la gestion des différentes fonctionnalités du système, y compris l'initialisation des données, l'archivage et le traitement des informations par les journalistes.

---

## Fichiers principaux

- **`initial.c`** : Initialise les données et structures nécessaires au système.
- **`archiviste.c`** : Gère les opérations d'archivage et communique avec les journalistes.
- **`journaliste.c`** : Traite les informations et simule le rôle des journalistes.
- **`types.h`** : Contient les définitions des types utilisés dans tout le projet.

---

## Technologies utilisées

- Programmation en C
- Gestion des processus (`fork()`, `exec()`)
- Communication entre processus (IPC)
- Sémaphores System V
- Mémoire partagée
- Files de messages

---

## Equipe projet 

- Nathan Thiery
- Ozan Gunes

# Note obtenue

Projet noté **14/20** à l'université Jean Monnet (année 2023-2024)
