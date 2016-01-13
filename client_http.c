#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h> 

int main(int argc, char *argv[]) { 
	char buffer[200],texte[200]; 
	int port, rc, sock,i,c, menu = 0,req_http=0; 
	struct sockaddr_in addr;
	struct hostent *entree;
    
	if(argc < 2) { 
    printf("\nUsage:\n client [option] [<command><sentence>...]\n\n");
    printf("Commands:\n +\t\tcount number of letter\n -\t\tcount value of letter\n\nOptions:\n -v\t\tuse verbose mode with menu\n\n");
		exit(1);
	}else{
		strcpy(texte,argv[1]);
	}
	
	char* requete_post;
	char* requete_get;
	addr.sin_port=htons(80); 
	addr.sin_family=AF_INET; 
	
	if(texte[0]=='&'){
		int taille_req_post=64;
		
		entree=(struct hostent *)gethostbyname("http://www.isima.fr/~laurenco/ZZ2/nb_letter_get.php");
		bcopy((char *)entree->h_addr,(char *)&addr.sin_addr,entree->h_length);
		sock= socket(AF_INET,SOCK_STREAM,0);
		puts("test");
		requete_post=malloc((sizeof(char)*taille_req_post)+sizeof(texte));
		
		strcpy(requete_post,"GET http://www.isima.fr/~laurenco/ZZ2/nb_lettre_get.php?phrase=");
		strcpy(requete_post+taille_req_post,texte+1);
		
		printf("message & done \n");
	}
	
	if(connect(sock, (struct sockaddr *)&addr,sizeof(struct sockaddr_in)) < 0) {
		printf("Connexion http problem!\n"); 
		exit(1);
	}
	printf("Connexion http established.\n");
	
	send(sock,texte,strlen(texte)+1,0);
  recv(sock,buffer,sizeof(buffer),0);
  printf("Server's message:\n>>> %s\n",buffer);
	
  bzero(texte,sizeof(texte));
  sprintf(texte,"/\0");
  send(sock,texte,strlen(texte)+1,0);    
	close(sock);

	return 0;
}
