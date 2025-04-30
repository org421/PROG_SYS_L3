#include "./types.h"


MemPartagee * themes;              //Nom de la memoire partagee 
int idMemPart;                     //Id de la memoire partagee
pid_t *liste_archivistes;          //Tableau des PID enfants archivitses
pid_t *liste_journalistes;         //Tableau des PID enfants journalistes
int nb_archivistes;   /* Nombre d'archivistes à créer donné par l'utilisateur (argv[1]) */
int nb_journalistes = 0;  // Nombre de journalistes
int semap;                // Nom de l'ensemble de sémaphore
unsigned short *val_init;  /* valeurs de l'es */



void killEnfants(pid_t parent_pid, int signal){
  //Kill les enfants
  int i;

  couleur(VERT); fprintf(stdout,"Parent : signal reçu [%d]\n", signal); couleur(REINIT);

  //Kill les enfants archivistes
  for(i = 0; i< nb_archivistes ; i++){
    kill(liste_archivistes[i], SIGINT);
  }

  //Detachement + suppression des IPCs
  //Suppression des files de messages
  couleur(VERT); fprintf(stdout,"Nombre de file de message à supprimer : %d\n", nb_archivistes); couleur(REINIT);
  for(i=0 ; i < nb_archivistes ; i++){
    msgctl(themes->id_file_messages[i], IPC_RMID, NULL);
    couleur(VERT); fprintf(stdout,"\tLa file [%d] a été supprimée\n", themes->id_file_messages[i]); couleur(REINIT);
  }

  //Suppression de la memoire partagee
  shmdt(themes);
  shmctl(idMemPart, IPC_RMID, NULL);

  //Liberer delivrer je ne mentirais plus jamais
  free(val_init);
  
  //Suppression de l'ensemble de semaphores
  semctl(semap, 1, IPC_RMID, NULL);

  couleur(VERT); fprintf(stdout,"Suicide du parent [%d]\n", parent_pid); couleur(REINIT);
  //Fin du programme
  couleur(REINIT);
  exit(EXIT_SUCCESS);
}


void gestionnaireSignal(int signal){
  pid_t parent_pid = getpid();
  killEnfants(parent_pid, signal);
}


void usage(char *argv[]){
  couleur(VERT); fprintf(stdout,"<%s> <nb_archivistes> <nb_themes>\n\tnb_archivistes >= 2\n\tnb_themes >= 3\n\n", argv[0]); couleur(REINIT);
  exit(EXIT_FAILURE);
}

/* Fonction qui renvoie l'id d'une file de message créée en fonction du i */
int creer_id_file_mess(int i){

  key_t clef = ftok(FICHIER_CLE, i);
  if(clef == -1){
    fprintf(stderr, "Err création Clef ftok\n");
    exit(EXIT_FAILURE);
  }

  int id_file_mess = msgget(clef, IPC_CREAT | 0660);
  if(id_file_mess == -1){
    fprintf(stderr, "Err id_file_mess initial.c\n");
    exit(EXIT_FAILURE);
  }

  //printf("Numéro clé message file : %d", id_file_mess);
  return id_file_mess;
}



int main(int argc, char* argv[]){
  int nb_themes;        /* Nombre de themes à créer donné par l'utilisateur (argv[2])     */
  int i, j, encore = 1;        /* -_- */
  pid_t archiviste_pid; /* Pid des archivistes */
  pid_t journaliste_pid; /* Pid des journalistes */
  int chance;          /* Calcul le pourcentage de chance pour décidé du type de requête des journalistes */
  char i_str[10], nb_themes_str[10]; // version char* des arguments que l'on donne au journalistes
  char numero_theme_str[10], numero_article_str[10], nombre_archiviste_str[10];  // version char* des arguments que l'on donne au journalistes
  char texte[4];  // Argument pour les journaliste, pour la création (texte d'un article)

  

  srand((unsigned int)time(NULL));
  
  if(argc < 3){
    usage(argv);
  }

  
  if ( ((nb_archivistes = atoi(argv[1])) < 2) || ((nb_themes = atoi(argv[2])) < 3) ){
    usage(argv);
  }

  // Allocation du bon nombre de semaphore pour val_init
  val_init = (unsigned short *) malloc ((5 * nb_themes )* sizeof(unsigned short));
  // Initialisation de val_init
  for(i = 0; i < (nb_themes * 5); i++){
    val_init[i] = 1;
  }
  
  couleur(VERT); fprintf(stdout,"\t\t\tPID Parent : [%d]\n\n\n", getpid()); couleur(REINIT);
  
  /* Creation de la cle :                                 */
  /* 1 - On teste si le fichier cle existe dans 
     le repertoire courant et on le fabrique si il n'existe pas
  */
  struct stat st;
  if((stat(FICHIER_CLE, &st) == -1) && (open(FICHIER_CLE, O_RDONLY | O_CREAT | O_EXCL, 0660) == -1) ){
    fprintf(stderr,"Pas de fichier clef et pb créa fichier clef, ciao\n");
    free(val_init);
    exit(EXIT_FAILURE);
  }


  key_t clef = ftok(FICHIER_CLE, LETTRE_CODE);
  if(clef == -1){
    fprintf(stderr, "Err création Clef ftok\n");
    free(val_init);
    exit(EXIT_FAILURE);
  }

  
  /*Création de la mem partagée pour le theme*/
  idMemPart = shmget(clef, (sizeof(MemPartagee)), IPC_CREAT | IPC_EXCL | 0660);
  if( idMemPart == -1){
    perror("shmget");
    fprintf(stderr, "Err idMemPart création shmget\n");
    free(val_init);
    exit(EXIT_FAILURE);
  }

  /*Attachement mem partagee*/
  themes = (MemPartagee*)shmat(idMemPart, NULL, 0);
  if(themes ==(MemPartagee *) -1){
    /*Si erreur, on supprime l'ipc*/
    perror("shmat");
    fprintf(stderr,"Err shmat theme1\n");
    shmctl(idMemPart, IPC_RMID,NULL);
    free(val_init);
    exit(EXIT_FAILURE);
  }

  // Création du sémaphore ECRITURE
  semap = semget(clef, 1, IPC_CREAT | IPC_EXCL | 0660);
  if(semap == -1){
    fprintf(stderr, "Problème de création du sémaphore dans initial (ou il existe deja) semap\n");
    shmdt(themes);
    shmctl(idMemPart, IPC_RMID, NULL);
    free(val_init);
    exit(EXIT_FAILURE);
  }

  //Initialisation du sémaphore ECRITURE
  if(semctl(semap, 1, SETALL, val_init) == -1){
    fprintf(stderr, "Problème init sémaphore dans initial.c semap\n");
    perror("init sem initial.c : ");
    semctl(semap, 1, IPC_RMID, NULL);
    shmdt(themes);
    shmctl(idMemPart, IPC_RMID, NULL);
    free(val_init);
    exit(EXIT_FAILURE);
  }

  

  // Initialisation du segment de mem
  for(i=0 ; i<NB_ARCHIVISTES_MAX ; i++){
    themes->pile_requetes[i] = 0;
    themes->id_file_messages[i] = 0;
  }
  for(i=0 ; i<THEMES_MAX ; i++){
    themes->theme_numero[i].nb_articles = 0;
    themes->theme_numero[i].nb_lecteurs = 0;
    themes->theme_numero[i].nb_ecrivains = 0;
  }
  themes->nb_themes = nb_themes;
  themes->nb_archivistes = nb_archivistes;


  /* Initialisation du gestionnaire des signaux */
  for(i = 1; i< 22 ;i++){
    if(i != 17) // SIGUSR2
      signal(i, gestionnaireSignal);
  }

  liste_archivistes = (pid_t *) malloc (nb_archivistes * sizeof(pid_t));

  /*On lance les nb_archivistes Archivistes*/
  couleur(VERT); fprintf(stdout,"\n\t\tLancement des [%d] archivistes ...\n\n", nb_archivistes); couleur(REINIT); 
  /* Mise en char* du nb_themes pour le passer en arg */
  snprintf(nb_themes_str, sizeof(nb_themes_str), "%d", nb_themes);
  for(i=0;i<nb_archivistes;i++){
    fprintf(stderr,".");
    archiviste_pid = fork();

    if(archiviste_pid == -1)
      break;
    
    
    /*Création pile de mess individuelle a l'archiviste*/
    themes->id_file_messages[i] = creer_id_file_mess(i);
    
    
    /*Archiviste créé*/
    if(archiviste_pid == 0){
      /*Stockage du pid dans le tableau*/
      liste_archivistes[i] = archiviste_pid;
      /* Mise en char* du i pour le passer en arg */
      snprintf(i_str, sizeof(i_str), "%d", i);
      execl("./archiviste", "archiviste", i_str, nb_themes_str, NULL);
      exit(EXIT_FAILURE);
    }
  }
  couleur(VERT); fprintf(stdout,"\n\n\t\tLes archivistes ont été créé ...\n"); couleur(REINIT); 

  /*Boucle infini sur journalistes*/
  couleur(VERT); fprintf(stdout,"\n\t\tLancement des journalistes ...\n\n"); couleur(REINIT); 
  liste_journalistes = (pid_t *) malloc ( sizeof(pid_t));

  
  while(encore){
    sleep(rand()%2);
    chance = (int) rand() % 10 +1;
    journaliste_pid = fork();

    if(journaliste_pid == -1)
      break;
    
    /* Journaliste créé */
    snprintf(nombre_archiviste_str, sizeof(nombre_archiviste_str), "%d", nb_archivistes);
    if(journaliste_pid == 0){

      /*Envoie du journaliste*/
     
      if(chance <= 7){	      /*Consultation*/
	snprintf(numero_theme_str, sizeof(numero_theme_str), "%d", rand() % nb_themes);
	snprintf(numero_article_str, sizeof(numero_article_str), "%d", rand() % 100);
	/*numéro theme concerné, numéro de l'article*/
	execl("./journaliste", "journaliste", nombre_archiviste_str, "C", numero_theme_str, numero_article_str, NULL);
      }
      if(chance == 9 || chance == 8){	      /*Publication*/
	snprintf(numero_theme_str, sizeof(numero_theme_str), "%d", rand() % nb_themes);
	for(j=0;j<4;j++){
	  texte[j] = (char) ('a' + rand() % 26);
	}
	texte[4] = '\0';
	/*numero theme concerné, texte de l'article (4 car)*/
	execl("./journaliste", "journaliste", nombre_archiviste_str, "P", numero_theme_str, texte, NULL);
      }
      if(chance > 9){	      /*Effacement*/
	snprintf(numero_theme_str, sizeof(numero_theme_str), "%d", rand() % nb_themes);
	snprintf(numero_article_str, sizeof(numero_article_str), "%d", rand() % 100);
	/*numéro theme concerné, numéro de l'article*/
	execl("./journaliste", "journaliste", nombre_archiviste_str, "E", numero_theme_str, nb_themes_str, NULL);
      }
    }
    
  }

  couleur(VERT); fprintf(stderr,"\t\tLes journalistes ont été créés\n\n"); couleur(REINIT); 

  
  for(i=0 ; i<NB_ARCHIVISTES_MAX ; i++){
    msgctl(themes->id_file_messages[i], IPC_RMID, NULL);
  }

  //Suppression des smp
  shmdt(themes);
  shmctl(idMemPart, IPC_RMID, NULL);
  //Suppression de l'ensemble de semaphores
  semctl(semap, 1, IPC_RMID, NULL);
  //Liberer delivrer je ne mentirais plus jamais
  free(val_init);
  
  exit(EXIT_SUCCESS);
}
