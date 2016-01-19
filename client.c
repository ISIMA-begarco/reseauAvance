#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h> 

#define DEBUG 0

int main(int argc, char *argv[]) {
	char buffer[200],texte[200],res_http[1024],requete_http[200],token[200],recep_msg[100];
	char add_serveur[200], chooseAddr[200];
	int port=-1, choosePort = -1, chooseHttp=0;
	
	//Caractere choisissant le traitement
	char char_consonant='+';
	char char_vowel='-';
	char char_number='?';
	char char_value='=';
	
	int sock,i,c, menu = 0, mode_http=0, resultat_traitement=-1;
	struct sockaddr_in addr;
	struct hostent *entree;

	if(argc == 2 && !strcmp(argv[1], "-v")) {
		menu = 1;
	} else if(argc > 2 && !strcmp(argv[1], "-o")) {
		chooseHttp=0;
	} else if(argc < 3) { 
		printf("\nUsage:\n client [option] <server address> <port> [<command><sentence>...]\n\n");
		printf("Commands:\n %c\t\tcount number of consonant\n"
		" %c\t\tcount number of vowel\n"
		" %c\t\tcount number of letter (address and port doesn\'t matter)\n"
		" %c\t\tcount value of the sentence (address and port doesn\'t matter)\n"
		"\nOptions:\n -v\t\tuse verbose mode with menu\n -o\t\tuse only http requests\n\n",
		char_consonant,char_vowel,char_number,char_value);
		exit(1);
	}
	int         nbCom = argc==3 || menu ? 1 : argc - 3,
		    	iteArg = 3;
		        
	while(nbCom) {
		mode_http=0;					// Pour eviter tout traitement non prevu
		
		bzero(texte,sizeof(texte));
		bzero(buffer,sizeof(buffer));      
		bzero(res_http,sizeof(res_http));      
		if(menu){                       // cas du menu
			int choice = 0;
			while(!choice) {
				printf("Possible operations:\n\t1- Consonant count\n\t2- Vowel count\n\t3- Characters count\n\t4- Value count\n\nYour choice?\n>>> ");
				bzero(texte,sizeof(texte));
				i = 0;
				while((c=getchar()) != '\n') 
					texte[i++]=c;
					choice = texte[0] - '0';
					choice = (choice > 0 && choice < 5) ? choice : 0;
			}
			bzero(texte,sizeof(texte));
			  
			// ajout du caractere de la commande
			switch(choice) {
				//Commandes ssh
				case 1:
				    texte[0] = char_consonant;i = 1;					
		  			break;
				case 2:
				    texte[0] = char_vowel;
				    break;
				    
				//Commandes http
				case 3:
				    texte[0] = char_number;
				    break;
				case 4:
				    texte[0] = char_value;
				    break;
				default:
					printf("Bad entry.\n");
				    break;
			}
			  
		  	printf("\nEnter your sentence:\n>>> "); 
		  	i = 1;
		  	while((c=getchar()) != '\n'){
		  	    texte[i++]=c;
		  	}
		  	
		} else if(argc==3) {                    // cas sans le menu
			i = 0;
		  	printf("Enter your command:\n>>> "); 
			while((c=getchar()) != '\n'){
				texte[i++]=c;
			}
		
		} else {                                // cas des commandes en ligne de commande
			  strcpy(texte, argv[iteArg++]);
		}
		
		//Test mode HTTP
		mode_http = texte[0]==char_number || texte[0]==char_value;
		
		if(!mode_http && !chooseHttp){							//Connection ssh
			texte[strlen(texte)]='\n';
			texte[strlen(texte)+1]='\0';
			
			if(menu) {
				printf("\nEnter server's address:\n>>> ");
				i = 0;
				while((c=getchar()) != '\n'){
					chooseAddr[i++]=c;
				}
				chooseAddr[i]='\0';
				printf("\nEnter server's port:\n>>> "); 
				scanf("%d", &choosePort);
			}
			
			port=(menu?choosePort:atoi(argv[2]));
			strcpy(add_serveur,(menu?chooseAddr:argv[1]));
		}else{									//Connection http
			port=80;
			strcpy(add_serveur,"www.isima.fr");
			
			if(texte[0]==char_number){
				// Requete GET => compteur de lettre
				sprintf(requete_http,"GET /~laurenco/ZZ2/nb_lettre_get.php?phrase=%s HTTP/1.1\r\n"
						"Host: www.isima.fr\r\n"
						"\r\n\r\n",texte+1);
						
				if(DEBUG) printf("Http request:\n%s",requete_http);
				
				strcpy(token,"Your sentence contains: ");
				strcpy(recep_msg,"Number of letters: ");
				
			}else if(texte[0]==char_value){
				// Requete POST => valeur phrase
				sprintf(requete_http,"POST /~laurenco/ZZ2/val_phrase_post.php HTTP/1.1\r\n"
					"Host: www.isima.fr\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n"
					"Content-Length: %d\r\n"
					"\r\n"
					"phrase=%s\r\n",(int)(7+strlen(texte))-1,texte+1);
					
				if(DEBUG) printf("Requete http :\n%s",requete_http);
				
				strcpy(token,"Value of sentence: ");
				strcpy(recep_msg,"Value of sentence: ");
				
			}else{
				printf("Attention erreur dans le mode HTTP\n reception d'un caractere %c\n",texte[0]);
			}
			
			// On donne a la variable d'envoie la requete http
			strcpy(texte,requete_http);
		}
	
		// Creation de la socket
		addr.sin_port=htons(port); 
		addr.sin_family=AF_INET; 
	
		entree=(struct hostent *)gethostbyname(add_serveur);
		if (entree==NULL){
			printf("Connexion host problem! %s unreachable\n",add_serveur);
			exit(1);
		}
			
		bcopy((char *)entree->h_addr,(char *)&addr.sin_addr,entree->h_length);
		sock= socket(AF_INET,SOCK_STREAM,0);
		if(connect(sock, (struct sockaddr *)&addr,sizeof(struct sockaddr_in)) < 0) {
			printf("Connexion problem!\n"); 
			exit(1);
		} 
		printf("Connexion established.\n");

		// Envoie
		if(send(sock,texte,strlen(texte),0)){

			// Reception
			if(!mode_http){
				recv(sock,buffer,sizeof(buffer),0);
				printf("Server's message:\n>>> %s\n",buffer);
			}else{
				while(recv(sock,buffer,sizeof(buffer),0)){
					strcat(res_http,buffer);
				}
				if(res_http==NULL){
					printf("Pas de reponse du serveur HTTP\n");
				}
				// Traitement de la reponse du serveur HTTP		
				if(DEBUG) printf("\n%s\n",res_http);
				
				resultat_traitement = -1;
				
				if(res_http!=NULL){
					// On parse le resultat renvoye par le serveur distant
					char* temp =strstr(res_http,token);
					strcat(token,"%d");
					sscanf(temp,token,&resultat_traitement);
					// On l'affiche
					printf("Server's message:\n>>>%s%d\n",recep_msg,resultat_traitement);
				}
			}
		}else{
			printf("Send error");
			exit(1);
		}

		nbCom--;
	}

	bzero(texte,sizeof(texte));
	strcpy(texte,"/\0");
	send(sock,texte,strlen(texte)+1,0);    
	//close(sock);

	return 0;
}
