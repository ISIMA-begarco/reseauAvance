#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct lettre {
	unsigned long v;
	unsigned long c;
} lettre;

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

int main(int argc, char * argv[]) {
	int s_ecoute, scom, lg_app, i, j; 
	struct sockaddr_in adr; 
	struct sockaddr_storage recep; 
	char buf[1500], renvoi[1500], host[1024],service[20];

    if(argc < 2) {
        printf("\nUsage:\n serveur <port>\n\n");
    } else {
	    s_ecoute=socket(AF_INET,SOCK_STREAM,0); 
	    printf("Creation de la socket.\n"); 
	    adr.sin_family=AF_INET; 
	    adr.sin_port=htons(atoi(argv[1])); 
	    adr.sin_addr.s_addr=INADDR_ANY; 
	
	    if(bind(s_ecoute,(struct sockaddr *)&adr,sizeof(struct sockaddr_in)) !=0) { 
		    printf("Probleme de bind sur v4\n"); 
		    exit(1);
	    } 

	    if(listen(s_ecoute,5) != 0) { 
		    printf("Probleme d'ecoute.\n"); exit(1);
	    } 

	    printf("En attente d'un client ...\n"); 
	
	    int         client = 0,
	                serveur = 1;
	
	    while(serveur) {
		    scom = accept(s_ecoute,(struct sockaddr *)&recep, (unsigned long *)&lg_app); 
		    getnameinfo((struct sockaddr *)&recep,sizeof(recep), host, sizeof(host),service,sizeof(service),0);
		    printf("Recu de %s venant du port %s\n",host, service); 
		    client = 1;
		    while(client) { 
			    recv(scom,buf,sizeof(buf),0); 
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
				    sprintf(renvoi, "Fin de la session\n>>> Arrêt du serveur\n");
			    } else {
				    sprintf(renvoi, "Commande inconnue !\n");
                }
			    send(scom,renvoi,strlen(renvoi),0);
		    }
		    close(scom);
	    } 
	    close(s_ecoute);
    } 
}
