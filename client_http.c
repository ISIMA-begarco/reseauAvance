#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h> 

#define DEBUG 0

int main(int argc, char *argv[]) { 
	char buffer[200],res[1024],texte[200],requete_http[200],token[200],char_number='#',char_value='=';
	char* add_serveur = "www.isima.fr";
	int sock=0; int resultat_traitement=-1;
	struct sockaddr_in addr;
	struct hostent *entree;
     
	if(argc < 2) { 
    printf("\nUsage:\n client [option] [<command><sentence>...]\n\n");
    printf("Commands:\n %c\t\tcount number of letter\n %c\t\tcount value of letter\n\nOptions:\n -v\t\tuse verbose mode with menu\n\n",char_number,char_value);
		exit(1);
	}else{
		strcpy(texte,argv[1]);
	}
	
	addr.sin_port=htons(80); 
	addr.sin_family=AF_INET; 
	
	entree=(struct hostent *)gethostbyname(add_serveur);
	if (entree==NULL){
		printf("Connexion host problem! %s unreachable\n",add_serveur);
		exit(1);
	}
	bcopy((char *)entree->h_addr,(char *)&addr.sin_addr,entree->h_length);
	sock= socket(AF_INET,SOCK_STREAM,0);
	
	if(connect(sock, (struct sockaddr *)&addr,sizeof(struct sockaddr_in)) < 0) {
		printf("Connexion http problem!\n"); 
		exit(1);
	}
	printf("Connexion http established.\n");

	if(texte[0]==char_number){
		sprintf(requete_http,"GET /~laurenco/ZZ2/nb_lettre_get.php?phrase=%s HTTP/1.1\r\n"
				"Host: www.isima.fr\r\n"
				"\r\n\r\n",texte+1);
		if(DEBUG) printf("Requete http :\n%s",requete_http);
		strcpy(token,"La phrase comporte : ");
	}

	if(texte[0]==char_value){
		sprintf(requete_http,"POST /~laurenco/ZZ2/val_phrase_post.php HTTP/1.1\r\n"
            "Host: www.isima.fr\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "phrase=%s\r\n",(int)(7+strlen(texte))-1,texte+1);
		if(DEBUG) printf("Requete http :\n%s",requete_http);
		strcpy(token,"La phrase a une valeur de : ");
	}

	if(send(sock,requete_http,strlen(requete_http),0)){
		printf("Server's message: >>>\n");
		while(recv(sock,buffer,sizeof(buffer),0)){
			strcat(res,buffer);
		}
		if(res==NULL){
			printf("Pas de reponse\n");
			exit(1);
		}
		
		if(DEBUG) printf("\n%s\n",res);
		
		char* temp =strstr(res,token);
		strcat(token,"%d");
		sscanf(temp,token,&resultat_traitement);
		
		printf("Resultat calcule a distance : %d\n",resultat_traitement);
		
	}else{
		printf("Send error");
		exit(1);
	}
	
  bzero(texte,sizeof(texte));
	return 0;
}
/*
POST /~laurenco/ZZ2/val_phrase_post.php HTTP/1.1 \r\n Host: www.isima.fr
Content-Length: 11
Content-Type: text/plain

phrase=caca

*/
