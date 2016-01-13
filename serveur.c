#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>


#define	NB_CLIENTS_MAX	2


int             serveur           = 0,
                maxClients        = NB_CLIENTS_MAX,
                s_ecoute          = 0, 
				nbClients         = 0;       
pthread_t     * clients           = NULL;
pthread_mutex_t mutex             = PTHREAD_MUTEX_INITIALIZER;

/*
 *
 *	Structure de retour de la fonction de compte
 *
 */
typedef struct lettre {
	unsigned long v;
	unsigned long c;
} lettre;

/*
 *
 *	Compte le nombre de voyelles et de consonnes
 *
 */
int findFreeThread() {
    int indice = 0;
    
    while(indice < maxClients && clients[indice]!=NULL) {
        indice++;
    }
    printf("indice %d\n", indice);
    fflush(stdin);
    return indice;
}

/*
 *
 *	Compte le nombre de voyelles et de consonnes
 *
 */
lettre countLetters(char * s) {
	lettre l;
	unsigned i = 0;

	l.v = 0;
	l.c = 0;
	
	while(s[i] != 0) {
		if(isalpha(s[i])) {
			if(strchr("eaiouyEAIOUY", s[i])) {
				(l.v)++;
			} else {
				(l.c)++;
			}
		}
		i++;
	}

	return l;
}

/*
 *
 *	Thread execute a l'arrivee d'un nouveau client
 *
 */
void * threadClient(void * args) {
	char        buf[1500], 
	            renvoi[1500];
	int         socket = *((int*) args),
	            numero = ((int*) args)[1],
                client = 1;
                printf("tread n°%d\n", numero);
                pthread_mutex_lock(&mutex); // verrouillage
                ((int*) args)[0] = NULL;
                pthread_mutex_unlock(&mutex); // deverrouillage
    while(client) {
	    usleep(1);   // on laisse la main
        recv(socket,buf,sizeof(buf),0);
	    printf("Message du client :\n>>> %s\n",buf);
	    lettre l = countLetters(buf);
	    bzero(renvoi,sizeof(renvoi));
	    if(buf[0]=='+')
		    sprintf(renvoi, "Nombre de consonnes : %d\n", l.c);
	    else if(buf[0]=='-')
		    sprintf(renvoi, "Nombre de voyelles : %d\n", l.v);
	    else if(buf[0]=='/') {
		    client = 0;				
		    sprintf(renvoi, "Fin de la session\n");
		    printf(renvoi, "Fin de la session\n");
	    } else if(buf[0]=='.') {
		    client = 0;
		    serveur = 0;
		    close(s_ecoute);// arret de l'ecoute
		    sprintf(renvoi, "Fin de la session\n>>> Arrêt du serveur\n");
	    } else {
		    sprintf(renvoi, "Commande inconnue !\n");
        }
	    send(socket,renvoi,strlen(renvoi),0);
    }
    nbClients--;
    clients[numero] = NULL;
    close(socket);
}

/*
 *
 *	Thread a l'ecoute de l'arrivee de nouveau clients
 *
 */
void * threadEcoute(void * args) {
	int    					    scom, 
							    lg_app;
	struct sockaddr_in 		    adr; 
	struct sockaddr_storage	    recep;

	char        	            host[1024],
	                            service[20];

    s_ecoute=socket(AF_INET,SOCK_STREAM,0); 
    printf("Creation de la socket.\n"); 
    adr.sin_family=AF_INET; 
    adr.sin_port=htons(*(int*)args); 
    adr.sin_addr.s_addr=INADDR_ANY; 

    if(bind(s_ecoute,(struct sockaddr *)&adr,sizeof(struct sockaddr_in)) !=0) { 
	    printf("Probleme de bind sur v4\n"); 
	    exit(1);
    } 

    if(listen(s_ecoute,5) != 0) { 
	    printf("Probleme d'ecoute.\n"); exit(1);
    } 
    
    //fcntl(s_ecoute, F_SETFL, O_NONBLOCK);       // rend la socket non bloquante
    
    printf("En attente d'un client ...\n"); 

    serveur = 1;
    int tab[2] = {0};
    while(serveur) {	// tant que le serveur n'a pas recu d'ordre d'extinction
		// on ecoute et allloue un thread
			if(nbClients < maxClients) {
				printf("nbClients : %d\n", nbClients);
					    printf("etat %d %d %d\n", clients[0], clients[1], clients[2]);
							scom = accept(s_ecoute,(struct sockaddr *)&recep, (unsigned long *)&lg_app);
							getnameinfo((struct sockaddr *)&recep,sizeof(recep), host, sizeof(host),service,sizeof(service),0);
							printf("Recu de %s venant du port %s.\n", host, service);
							int indice = findFreeThread();
							if(indice < maxClients) {
							    tab[0] = scom;
							    tab[1] = indice;
							    if( pthread_create( &clients[indice], NULL, threadClient, (void*) tab) < 0 ) {
							        perror("Impossible de creer le thread client.\n");
							    } else {
							        nbClients++;
							    }
				      } else {
	                printf("Client refuse.\n");
	                close(scom);
				      }
				}
			  usleep(1);
		 }
}

/*
 *
 *	Programme serveur multi-threads
 *
 */
int main(int argc, char * argv[]) {
	if(argc < 2) {
        printf("\nUsage:\n serveur <port> [NB_MAX_CLIENTS]\n\n");
  } else {
	  if(argc > 2)
		  maxClients = strtoul(argv[2], 0, 0);
		clients = (pthread_t *)malloc(sizeof(pthread_t)*maxClients);		// allocation des threads
		pthread_t ecoute;
		int port = atoi(argv[1]);
		if( pthread_create( &ecoute, NULL, threadEcoute, (void*)&port) < 0 ) {
	        perror("Impossible de creer le thread d'ecoute'.\n");
	    }
	    
	    serveur = 1;
	    while(serveur) {
	        usleep(1);
	    }
	    
	    close(s_ecoute);            // arret du socket d'ecoute
	    pthread_cancel(ecoute);     // arret du thread d'ecoute
	    
		int i = 0;
		for(i = 0 ; i < maxClients ; ++i) {
            if(clients[i] != 0)
    			pthread_join(clients[i], 0); // attente
	}
		
	free(clients);	// liberation des threads
  }
  return 0;
}
