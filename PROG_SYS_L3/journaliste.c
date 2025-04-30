#include "./types.h"


MemPartagee * themes;    // Nom de la mem partagee


void gestionnaireSignal(int signal) {
  couleur(BLEU); fprintf(stdout,"Journaliste : Signal capté, arrêt du programme\n"); couleur(REINIT);
  exit(EXIT_SUCCESS);
}



int main(int argc, char* argv[]){
  struct stat st;    
  requete_journaliste requete;   // Pour envoyer le message à l'archiviste
  reponse_archiviste reponse;   // Pour recevoir le message de l'archiviste
  key_t cle;  //  cle du smp
  int i; // For

  for(i = 0; i< 20 ;i++){
    signal(i, gestionnaireSignal);
  }
  
  if (stat(FICHIER_CLE, &st) == -1){
    fprintf(stderr, "Err stat archiviste numéro [%s]\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  cle = ftok(FICHIER_CLE, LETTRE_CODE);
  if(cle == -1){
    fprintf(stderr, "Err cle archiviste numéro [%s]\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  
  /* Récupération de la mem partagee */
  int idMemPart = shmget(cle, sizeof(MemPartagee), 0);
  if( idMemPart == -1){
    fprintf(stderr, "Err idMemPart création shmget journaliste (%s)\n", FICHIER_CLE);
    perror("Err idMemPart Journaliste");
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
  
  switch(argv[2][0]){
  case 'C' :
    // Cration de la roquête
    requete.type = 6598;
    requete.op = 'C';
    requete.num_theme = atoi(argv[3]);
    requete.num_article = atoi(argv[4]);
    break;
  case 'P' :
    // Cration de la roquête 
    requete.type = 6598;
    requete.op = 'P';
    requete.num_theme = atoi(argv[3]);
    requete.texte[0] = argv[4][0];
    requete.texte[1] = argv[4][1];
    requete.texte[2] = argv[4][2];
    requete.texte[3] = argv[4][3];
    break;
  case 'E' :
    // Cration de la roquête 
    requete.type = 6598;
    requete.op = 'E';
    requete.num_theme = atoi(argv[3]);
    requete.num_article = atoi(argv[4]);
    break;
  default:
    fprintf(stderr,"case default dans journaliste\n");
  }

  //Récupération de la prio la plus haute
  int min = 1000;
  int indice = -1;
  for(i=0; i< themes->nb_archivistes ; i++){
    if(themes->pile_requetes[i] < min){
      min = themes->pile_requetes[i];
      indice = i;
    }
  }

  //Envoie de la requête
  couleur(BLEU); fprintf(stdout,"(Journaliste) : Envoie sur la file de message numéro %d\n", themes->id_file_messages[indice]); couleur(REINIT); 
  themes->pile_requetes[indice]++; /////////
  if (msgsnd(themes->id_file_messages[indice], &requete, sizeof(requete), 6598) == -1){
    fprintf(stderr, "Err envoie de la requete journaliste\n");
    exit(EXIT_FAILURE);
  }

  //Attente de la réponse
  if(msgrcv(themes->id_file_messages[indice], &reponse, sizeof(reponse), 6599, 0) == -1){
    fprintf(stderr, "Err journaliste reception du message de l'archiviste.\n");
    exit(EXIT_FAILURE);
  }

  //Affichage du résultat
  couleur(BLEU);fprintf(stderr,"(Journaliste) : Réponse recue %s\n", reponse.reponse); couleur(REINIT); 
  
  shmdt(themes);
  
  exit(EXIT_SUCCESS);
}
