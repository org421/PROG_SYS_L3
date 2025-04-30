#include "./types.h"

MemPartagee * themes;

void gestionnaireSignal(int signal) {
  couleur(ROUGE); fprintf(stdout,"Archiviste : Signal capté, numéro du signal : [%d] -> Arrêt du programme\n", signal); couleur(REINIT);
  shmdt(themes);
  exit(EXIT_SUCCESS);
}


int main(int argc, char* argv[]){
  struct stat st;
  key_t cle;
  
  int continuer = 1;    // Pour la boucle while
  requete_journaliste requete;    // Pour receptionner le message
  reponse_archiviste reponse;     // Pour envoyer le message
  
  requete_journaliste file_attente[FILE_ATTENTE_MAX];    // Pour stocker les requetes reçues 
  int last_case = 0;   // Indice de la dernière case du tableau
  int nb_art;
  int i, j;            // Pour les boucles

  int semap;   /* nom de l'ensemble de sémaphores */
  
  struct sembuf P = {0, -1, SEM_UNDO};  // Opération P
  struct sembuf V = {0, 1, SEM_UNDO};   // Opération V 
  
  
  srand((unsigned int)time(NULL));
  
  couleur(ROUGE); fprintf(stdout,"\tJe suis l'archiviste [%s] et mon PID est : %d\n",argv[1], getpid()); couleur(REINIT);
  
  if (stat(FICHIER_CLE, &st) == -1){
    fprintf(stderr, "Err stat archiviste numéro [%s]\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  
  //Récuperation des cles
  cle = ftok(FICHIER_CLE, LETTRE_CODE);
  if(cle == -1){
    fprintf(stderr, "Err cle archiviste numéro [%s]\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  

  /* Récupération de la mem partagee */
  int idMemPart = shmget(cle, sizeof(MemPartagee), 0);
  if( idMemPart == -1){
    fprintf(stderr, "Err idMemPart création shmget archiviste numéro [%s]\n", argv[1]);
    perror("idMemPart shmget archiviste :");
    exit(EXIT_FAILURE);
  }

  /*Attachement mem partagee*/
  themes = (MemPartagee *) shmat(idMemPart, NULL, 0);
  if(themes ==(MemPartagee *) -1){
    /*Si erreur, on supprime l'ipc*/
    fprintf(stderr,"Err shmat theme1 archiviste numéro [%s]\n", argv[1]);
    shmctl(idMemPart, IPC_RMID,NULL);
    exit(EXIT_FAILURE);
  }

  // Récupération de l'ensemble de sémaphores SEMAP
  semap = semget(cle, 1, 0);
  if(semap == -1){
    fprintf(stderr,"(Archiviste) [%s] : Problème de récupération de sémaphore \n", argv[1]);
    shmdt(themes);
    exit(EXIT_FAILURE);
  }
  
  
  /* Initialisation des signaux*/
  for(i = 0; i< 20 ;i++){
    if(i != 2)
      signal(i, gestionnaireSignal);
  }

  //Initialisation du segment mem pour cet archiviste
  
  while(continuer){
    /* Traitement des opés */

    // Réception du travail
    if( msgrcv(themes->id_file_messages[atoi(argv[1])], &requete, sizeof(requete), 6598, 0) == -1){
      fprintf(stderr, "Err archiviste reception du message du journaliste.\n");
      exit(EXIT_FAILURE);
    }

    // Placement file d'attente
    file_attente[last_case] = requete;
    last_case++;
    
    // Execution du travail (penser aux sémaphores)
    if(last_case != 0){
      switch(file_attente[0].op){
      case 'C': // Consultation
	semop(semap,&P,(file_attente[0].num_theme * 5 +2) );
	semop(semap,&P,(file_attente[0].num_theme * 5 +1) );
	semop(semap,&P,(file_attente[0].num_theme * 5 +4) );
	
	themes->theme_numero[i].nb_lecteurs++;

	if(themes->theme_numero[i].nb_lecteurs == 1)
	  semop(semap,&P,(file_attente[0].num_theme * 5) );

	semop(semap,&V,(file_attente[0].num_theme * 5 +4) );
	semop(semap,&V,(file_attente[0].num_theme * 5 +1) );
	semop(semap,&V,(file_attente[0].num_theme * 5 +2) );
		
	couleur(ROUGE); fprintf(stdout,"\n(Archiviste) [%s] : Requête reçue :\n\t - [OP] : %c\n\t - [Num theme] : %d\n\t - [Num article] : %d\n\n\n", argv[1], requete.op, file_attente[0].num_theme, file_attente[0].num_article); couleur(REINIT);
	if(file_attente[0].num_article > themes->theme_numero[file_attente[0].num_theme].nb_articles){
	  strcpy(reponse.reponse, "Attention, cet article n'existe pas");
	}else{
	  // On met dans la réponse la chaine de caractere présente dans l'article demandé
	  strcpy(reponse.reponse, themes->theme_numero[file_attente[0].num_theme].article[file_attente[0].num_article]);
	}
	couleur(ROUGE); fprintf(stdout,"Sortie\n\n"); couleur(REINIT);

	semop(semap,&P,(file_attente[0].num_theme * 5 +4) );

	themes->theme_numero[i].nb_lecteurs--;

	if(themes->theme_numero[i].nb_lecteurs == 0)
	  semop(semap,&V,(file_attente[0].num_theme * 5) );

	semop(semap,&V,(file_attente[0].num_theme * 5 +4) );
	
	sleep(rand()%3);
	
	break;

      case 'P': // Créer l'article

	semop(semap,&P,(file_attente[0].num_theme * 5 +3) );
	themes->theme_numero[i].nb_ecrivains++;
	if(themes->theme_numero[i].nb_ecrivains == 1)
	  semop(semap,&P,(file_attente[0].num_theme * 5 +1));

	semop(semap,&V,(file_attente[0].num_theme * 5 +3));
	semop(semap,&P,(file_attente[0].num_theme * 5 ));
	
	couleur(ROUGE); fprintf(stdout,"\n(Archiviste) [%s] : Requête reçue :\n\t - [OP] : %c\n\t - [Num theme] : %d\n\t - [Texte de l'article à créer] : %c%c%c%c\n\n\n", argv[1], requete.op, file_attente[0].num_theme, file_attente[0].texte[0], file_attente[0].texte[1], file_attente[0].texte[2], file_attente[0].texte[3]); couleur(REINIT);
	nb_art = themes->theme_numero[file_attente[0].num_theme].nb_articles;
	strcpy(themes->theme_numero[file_attente[0].num_theme].article[nb_art] , file_attente[0].texte);
	themes->theme_numero[file_attente[0].num_theme].nb_articles ++;
      
	strcpy(reponse.reponse, "article créé");
	couleur(ROUGE); fprintf(stdout,"Sortie\n\n"); couleur(REINIT);

	semop(semap,&V,(file_attente[0].num_theme * 5 ));
	semop(semap,&P,(file_attente[0].num_theme * 5 + 3));

	themes->theme_numero[i].nb_ecrivains--;
	if(themes->theme_numero[i].nb_ecrivains == 0)
	  semop(semap,&V,(file_attente[0].num_theme * 5 + 1));

	semop(semap,&V,(file_attente[0].num_theme * 5 + 3));

	sleep(rand()%3);
	
	break;

      case 'E': // Effacer l'article

	semop(semap,&P,(file_attente[0].num_theme * 5 +3) );
	themes->theme_numero[i].nb_ecrivains++;
	if(themes->theme_numero[i].nb_ecrivains == 1)
	  semop(semap,&P,(file_attente[0].num_theme * 5 +1));

	semop(semap,&V,(file_attente[0].num_theme * 5 +3));
	semop(semap,&P,(file_attente[0].num_theme * 5 ));
	
	
	couleur(ROUGE); fprintf(stdout,"\n(Archiviste) [%s] : Requête reçue :\n\t - [OP] : %c\n\t - [Num theme] : %d\n\t - [Num article] : %d\n\n\n", argv[1], requete.op, file_attente[0].num_theme, file_attente[0].num_article); couleur(REINIT);
	if(file_attente[0].num_article > themes->theme_numero[file_attente[0].num_theme].nb_articles){
	  strcpy(reponse.reponse, "Attention, cet article n'existe pas");
	}else{
	  nb_art = themes->theme_numero[file_attente[0].num_theme].nb_articles;
	  themes->theme_numero[file_attente[0].num_theme].article[file_attente[0].num_article][0] = themes->theme_numero[file_attente[0].num_theme].article[nb_art][0];
	  themes->theme_numero[file_attente[0].num_theme].article[file_attente[1].num_article][1] = themes->theme_numero[file_attente[0].num_theme].article[nb_art][1];
	  themes->theme_numero[file_attente[0].num_theme].article[file_attente[2].num_article][2] = themes->theme_numero[file_attente[0].num_theme].article[nb_art][2];
	  themes->theme_numero[file_attente[0].num_theme].article[file_attente[3].num_article][3] = themes->theme_numero[file_attente[0].num_theme].article[nb_art][3];
	  themes->theme_numero[file_attente[0].num_theme].nb_articles --;
	  strcpy(reponse.reponse, "article effacé");
	}
	couleur(ROUGE); fprintf(stdout,"Sortie\n\n"); couleur(REINIT);


	semop(semap,&V,(file_attente[0].num_theme * 5 ));
	semop(semap,&P,(file_attente[0].num_theme * 5 + 3));

	themes->theme_numero[i].nb_ecrivains--;
	if(themes->theme_numero[i].nb_ecrivains == 0)
	  semop(semap,&V,(file_attente[0].num_theme * 5 + 1));

	semop(semap,&V,(file_attente[0].num_theme * 5 + 3));

	
	sleep(rand()%3);
	
	break;
      default:
	fprintf(stderr,"Op non reconnue par l'archiviste.\n");
      }
    }

    // Message notif au journaliste
    couleur(ROUGE); fprintf(stdout,"(Archiviste) [%s] : Envoie de la réponse <%s> au journaliste sur la file de message numéro %d.\n", argv[1], reponse.reponse, themes->id_file_messages[atoi(argv[1])]); couleur(REINIT);
    reponse.type = 6599;
    if (msgsnd(themes->id_file_messages[atoi(argv[1])], &reponse, sizeof(reponse), 6599) == -1){
      fprintf(stderr, "Err envoie de la reponse archiviste\n");
      perror("msgsend : ");
      exit(EXIT_FAILURE);
    }

    // On met a jour le nombre de requetes a traiter pour cet archivitse puisque l'on en a traité une
    themes->pile_requetes[atoi(argv[1])]--;
    // On supprime la requête traitée
    for(i = 0 ;  i < (last_case -1) ; i++){
      file_attente[i] = file_attente[i+1];
    }
    // On met à jour l'indice de la derniere case du tableau des requetes a traiter
    last_case --;

    // Affichage du segment de mémoire après le travail
    couleur(JAUNE); fprintf(stdout,"(Archiviste) [%s] : segment de mémoire :\n\t - Pile des requetes : %d\n\t - liste de themes : \n", argv[1], themes->pile_requetes[atoi(argv[1])]); couleur(REINIT);
    for(i = 0; i < themes->nb_themes ; i++){
      couleur(JAUNE); fprintf(stdout,"\t\t[Theme %d] : \n", i); couleur(REINIT);
      for(j = 0 ; j < themes->theme_numero[i].nb_articles ; j++){
	couleur(JAUNE); fprintf(stdout,"\t\t\t[Article %d] : %c%c%c%c\n", j, themes->theme_numero[i].article[j][0], themes->theme_numero[i].article[j][1], themes->theme_numero[i].article[j][2], themes->theme_numero[i].article[j][3]); couleur(REINIT);
      }
      fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n\n");
  
  }
  

  /*détachement smp*/
  shmdt(themes);
  
  
  exit(EXIT_SUCCESS);
}
