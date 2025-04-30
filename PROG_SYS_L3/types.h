#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>       
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <time.h>


#define THEMES_MAX 1024
#define STRING_MAX 1024
#define JOURNALISTES_MAX 1024
#define NB_ARCHIVISTES_MAX  1024
#define FILE_ATTENTE_MAX  1024

/* Informations contenues dans un thème */
typedef struct {
  int nb_articles;
  int nb_lecteurs;
  int nb_ecrivains;
  char article[100][4];
}theme;

/* Liste des themes et adresse des file de message */
typedef struct {
  theme theme_numero[THEMES_MAX];
  int nb_themes;
  int nb_archivistes;
  int id_file_messages[NB_ARCHIVISTES_MAX];
  int pile_requetes[NB_ARCHIVISTES_MAX];
}MemPartagee;

/* Informations contenues dans la requete d'un journaliste */
typedef struct {
  long type;
  int num_archiviste;
  int num_theme;
  int num_article;
  char texte[4];
  char op;
}requete_journaliste;

/* Informations contenues dans la requete d'un archiviste  */
typedef struct {
  long type;
  char reponse[STRING_MAX];
}reponse_archiviste;

#define FICHIER_CLE "cle.serv"

#define LETTRE_CODE 'a'
#define LETTRE_CODE1 'b'
#define LETTRE_CODE2 'c'
#define LETTRE_CODE3 'd'
#define LETTRE_CODE4 'e'


/* Couleurs dans xterm */
#define couleur(param) fprintf(stdout,"\033[%sm",param)

#define ROUGE "31"
#define VERT  "32"
#define JAUNE "33"
#define BLEU  "34"
#define REINIT "0"


/* 
   Utilisation (pour ecrire en rouge) :
   
   couleur(ROUGE); fprintf(stdout,"Hello\n"); couleur(REINIT);
 
*/

