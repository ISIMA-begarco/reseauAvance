#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h> 
#include <openssl/ssl.h>
#include <openssl/err.h>

#define DEBUG 0

//To install openssl
//sudo apt-get install libssl-dev

// declaration de fonctions utiles
SSL_CTX* InitCTX(void);
void ShowCerts(SSL* ssl);
int OpenConnection(const char *hostname, int port);


int main(int argc, char *argv[]) {
	char buffer[1024],texte[200],res_http[1024],requete_http[200],token[200],recep_msg[100];
	char add_serveur[200], chooseAddr[200];
	int port=-1, choosePort = -1, chooseHttp=0;
	
	SSL *ssl;
  	SSL_CTX *ctx;
 	int s_com, bytes;
	
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
		chooseHttp=1;
		if(DEBUG) printf("ONLY HTTP\n");
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
	int         nbCom = (argc==3 || menu) ? (chooseHttp?argc-2:1) : (chooseHttp?argc-2:argc - 3),
		    	iteArg = (chooseHttp?2:3);
	if(DEBUG) printf("Number of commands: %d %d\n", nbCom, iteArg);
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
		  	
		} else if(argc==3 && !chooseHttp) {                    // cas sans le menu
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
				if(!scanf("%d", &choosePort)){
					printf("Scanf error");
					exit(1);
				}
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
				
				strcpy(token,"La phrase comporte : ");
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
				
				strcpy(token,"La phrase a une valeur de : ");
				strcpy(recep_msg,"Value of sentence: ");
				
			}else{
				printf("Warning, Error in HTTP mode\nreception of character %c\n",texte[0]);
			}
			
			// On donne a la variable d'envoie la requete http
			strcpy(texte,requete_http);
		}
		
		//Preparation de l'envoie
		if(!mode_http){
			if (SSL_library_init() < 0){
				printf("initialisation Openssl library problem\n") ;
				exit(1);
			}
			ctx= InitCTX();
			s_com=OpenConnection(add_serveur, port);
			ssl = SSL_new(ctx);    // create new SSL connection state
			SSL_set_fd(ssl, s_com );    // attach the socket descriptor 
			if ( SSL_connect(ssl) == -1   ){   //perform the connection 
				ERR_print_errors_fp(stderr);
				exit(1);
			}
			printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
			
			ShowCerts(ssl);    
			
			// Envoie
			SSL_write(ssl, texte, strlen(texte));

			// Reception
			bytes = SSL_read(ssl, buffer, sizeof(buffer)); // reponse
			printf("Server's message:\n>>> %s\n",buffer);
			
			//Fermeture de la session
			bzero(texte,sizeof(texte));
			strcpy(texte,"/\0");
			SSL_write(ssl, texte, strlen(texte));
			
			buffer[bytes] = 0;
			SSL_free(ssl);        						// release connection state
		//	close(s_com );         // close socket 
   			SSL_CTX_free(ctx);        //release context 
		}else{
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
				/*while(recv(sock,buffer,sizeof(buffer),0)){
					strcat(res_http,buffer);
				}*/
				recv(sock,buffer,sizeof(buffer),0);
				strcpy(res_http,buffer);
				if(res_http==NULL){
					printf("No HTTP server response\n");
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
			}else{
				printf("Send error");
				exit(1);
			}
		}
		nbCom--;
	}

	//close(sock);

	return 0;
}








SSL_CTX* InitCTX(void) {   
	const SSL_METHOD *method;
    SSL_CTX *ctx;
 
    OpenSSL_add_all_algorithms();  /* load & register all cryptos. */
    SSL_load_error_strings();   /* load all error messages */
    method = SSLv3_client_method();  
	/* new server/client -method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL ) {
    	ERR_print_errors_fp(stderr);
        abort(); 
    }
    return ctx;
}

void ShowCerts(SSL* ssl) {   
	X509 *cert;
    char *line;
 
    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else
        printf("Aucun certificat.\n");
}



/// Fichier client_ssl.c pour ipv4
/*int main(int argc, char *argv[]) {
  SSL *ssl;
  SSL_CTX *ctx;
  int s_com, bytes;
  char buf[1024];

  if (argc !=3)  {
    printf("usage : client   nom_serveur port\n");
        exit(1);    }
    
  if (SSL_library_init() < 0)
    printf("probleme initialisation Openssl library\n") ;
  ctx= InitCTX();
  s_com=OpenConnection(argv[1], atoi(argv[2]));
  ssl = SSL_new(ctx);    // create new SSL connection state
  SSL_set_fd(ssl, s_com );    // attach the socket descriptor 
  if ( SSL_connect(ssl) == -1   )   //perform the connection 
        ERR_print_errors_fp(stderr);
    else
    {   char *msg = "Hello";
         printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        ShowCerts(ssl);        //get any certs 
        SSL_write(ssl, msg, strlen(msg));   //encrypt & send message 
        bytes = SSL_read(ssl, buf, sizeof(buf));// reponse 
        buf[bytes] = 0;
        printf("Received: \"%s\"\n", buf);
        SSL_free(ssl);        //release connection state 
    }
    close(s_com );         // close socket 
    SSL_CTX_free(ctx);        //release context 
    return 0;}*/

int OpenConnection(const char *hostname, int port) {   
	int sd;
    struct hostent *host;
    struct sockaddr_in addr;
 
    if ( (host = gethostbyname(hostname)) == NULL )
    {
        perror(hostname);
        abort();
    }
    sd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);
    if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        //close(sd);
        perror(hostname);
        abort();
    }
    return sd;
}

