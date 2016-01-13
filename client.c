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
    
  if(argc > 2 && !strcmp(argv[1], "-v")) {
        menu = 1;
	} else if(argc < 3) { 
        printf("\nUsage:\n client [option] <port> [<command><sentence>...]\n\n");
        printf("Commands:\n +\t\tcount number of consonant\n -\t\tcount number of vowel\n\nOptions:\n -v\t\tuse verbose mode with menu\n\n");
		exit(1);
	}

	addr.sin_port=htons(atoi(argv[(menu?3:2)])); 
	addr.sin_family=AF_INET; 
	entree=(struct hostent *)gethostbyname(argv[(menu?2:1)]);
	bcopy((char *)entree->h_addr,(char *)&addr.sin_addr,entree->h_length);
	sock= socket(AF_INET,SOCK_STREAM,0);
	if(connect(sock, (struct sockaddr *)&addr,sizeof(struct sockaddr_in)) < 0) {
		printf("Connexion problem!\n"); 
		exit(1);
	} 
	printf("Connexion established.\n");

  int         nbCom = argc==3 || menu ? 1 : argc - 3,
              iteArg = 3;
                
  while(nbCom) {
	  req_http=0;
    bzero(texte,sizeof(texte));
    bzero(buffer,sizeof(buffer));      
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
        		//Commande ssh
            case 1:
                texte[0] = '+';
          			break;
            case 2:
                texte[0] = '-';
                break;
                
            //Commande http
            case 3:
                texte[0] = '&';
                req_http=1;
                break;
            case 4:
                texte[0] = '|';
                req_http=1;
                break;
            default:
            		printf("Mauvaise saisie\n");
                break;
        }
          
      	printf("\nEnter your sentence:\n>>> "); 
      	i = 1;
      	while((c=getchar()) != '\n'){
      	    texte[i++]=c;
      	}
      	
    } else if(argc==3) {                    // cas sans le menu
        i = 0;
      	printf("Enter your command:\n>>> ",argc); 
        while((c=getchar()) != '\n'){
        	texte[i++]=c;
        }
    } else {                                // cas des commandes en ligne de commande
          strcpy(texte, argv[iteArg++]);
    }
    if(!req_http){
		  send(sock,texte,strlen(texte)+1,0);
		  recv(sock,buffer,sizeof(buffer),0);
		  printf("Server's message:\n>>> %s\n",buffer);
		  
		}else{
			char* requete_post;
			char* requete_get;
			addr.sin_port=htons(atoi(80)); 
			addr.sin_family=AF_INET; 
			
			if(texte[0]='&'){
				int taille_req_post=64;
			
				entree=(struct hostent *)gethostbyname("http://www.isima.fr/~laurenco/ZZ2/nb_letter_get.php");
				bcopy((char *)entree->h_addr,(char *)&addr.sin_addr,entree->h_length);
				sock= socket(AF_INET,SOCK_STREAM,0);
				
				requete_post=malloc((sizeof(char)*taille_req_post)+sizeof(texte));
				
				strcpy(requete_post,"GET http://www.isima.fr/~laurenco/ZZ2/nb_lettre_get.php?phrase=");
				strcpy(requete_post+taille_req_post,texte);
								
			} else { //text[0]='|'
				entree=(struct hostent *)gethostbyname("http://www.isima.fr/~laurenco/ZZ2/val_phrase_post.php");
				bcopy((char *)entree->h_addr,(char *)&addr.sin_addr,entree->h_length);
				sock= socket(AF_INET,SOCK_STREAM,0);
				
			}
			
			if(connect(sock, (struct sockaddr *)&addr,sizeof(struct sockaddr_in)) < 0) {
				printf("Connexion http problem!\n"); 
				exit(1);
			}
			printf("Connexion http established.\n");
			
			send(sock,texte,strlen(texte)+1,0);
		  recv(sock,buffer,sizeof(buffer),0);
		  printf("Server's message:\n>>> %s\n",buffer);
		}
  	nbCom--;
  }
  
  bzero(texte,sizeof(texte));
  sprintf(texte,"/\0");
  send(sock,texte,strlen(texte)+1,0);    
	close(sock);

	return 0;
}
