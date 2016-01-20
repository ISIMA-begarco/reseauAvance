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
#include <openssl/ssl.h>
#include <openssl/err.h>


#define	NB_CLIENTS_MAX	1024


/// declaration de fonctions utiles
SSL_CTX* InitCTX(void);
void ShowCerts(SSL* ssl);
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);
void Servlet(SSL* ssl);
int OpenListener(int port);
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);

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

    while(indice < maxClients && clients[indice]!=0) {
        indice++;
    }

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
	int         numero = *((int**) args)[1],
				client = 1;
	int sd, bytes;
	
	SSL* ssl = ((SSL**)args)[0];
	
	if ( SSL_accept(ssl) == -1 )  //do SSL-protocol accept
        ERR_print_errors_fp(stderr);
    else{
		while(client) {
			usleep(1);   // on laisse la main
			ShowCerts(ssl);        // get any certificates du client
			bytes = SSL_read(ssl, buf, sizeof(buf)); // lecture
			printf("lecture de %d bytes\n",bytes);
			if ( bytes > 0 )
			{
				buf[bytes] = 0;
				printf("Client msg: \"%s\"\n", buf);
				
				lettre l = countLetters(buf);
				bzero(renvoi,sizeof(renvoi));
				if(buf[0]=='+')
					sprintf(renvoi, "Number of consonants: %lu\n", l.c);
				else if(buf[0]=='-')
					sprintf(renvoi, "Number of vowels: %lu\n", l.v);
				else if(buf[0]=='/') {
					client = 0;
					sprintf(renvoi, "Session ended.\n");
					printf(renvoi, "Session ended.\n");
				} else if(buf[0]=='.') {
					client = 0;
					serveur = 0;
					close(s_ecoute);// arret de l'ecoute
					sprintf(renvoi, "Session ended.\n>>> Server stopped.\n");
				} else {
					sprintf(renvoi, "Unknown request!\n");
				}
				SSL_write(ssl, renvoi, strlen(renvoi));// envoie
			}else{
           		ERR_print_errors_fp(stderr);
           	}
		}
	}
	sd = SSL_get_fd(ssl);       // get socket connection 
    SSL_free(ssl);         // release SSL state
    close(sd);          // close connection*/
   
   	nbClients--;
	clients[numero] = 0;
	pthread_exit(NULL);
}

/*
 *
 *	Thread a l'ecoute de l'arrivee de nouveau clients
 *
 */
void * threadEcoute(void * args) {
	int 	scom,
		//	lg_app,
			s_ecoute;
	struct sockaddr_in 		    appelant;
	
	socklen_t len = sizeof(appelant); 

	//char        	            host[1024],
	//                          service[20];
	                          
	SSL_CTX *ctx;
	SSL *ssl;
	int portnum = *((int*)args);

	SSL_library_init();   // initialisation librairie ssl
	ctx= InitCTX();  // initialisation context SSL -> fct 
	LoadCertificates(ctx, "server.crt", "server.key"); 
	//load certs
	s_ecoute = OpenListener(portnum);	
	
	//fcntl(s_ecoute, F_SETFL, O_NONBLOCK);       // rend la socket non bloquante
	
	printf("Waiting for client...\n"); 

	serveur = 1;
	void* tab[2] = {0};
	while(serveur) {	// tant que le serveur n'a pas recu d'ordre d'extinction
	// on ecoute et allloue un thread
		if(nbClients < maxClients) {
			scom = accept(s_ecoute, (struct sockaddr*)&appelant,&len);
			ssl = SSL_new(ctx);   // get new ssl state with context 
			SSL_set_fd(ssl, scom);   /// set connection socket to SSL state
			int indice = findFreeThread();
			if(indice < maxClients) {
					tab[0] = ssl;
					tab[1] = &indice;
					if( pthread_create( &clients[indice], NULL, threadClient, (void*) tab) < 0 ) {
							perror("Cannot create client thread.\n");
					} else {
							nbClients++;
					}
			} else {
					printf("Client refused.\n");
					close(scom);
			}
		}
		usleep(1);
	}
	close(s_ecoute);          
	SSL_CTX_free(ctx);    
	pthread_exit(NULL);
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
	  	if(argc > 2){
		  maxClients = strtoul(argv[2], 0, 0);
		}
		clients = (pthread_t *)malloc(sizeof(pthread_t)*maxClients);		// allocation des threads
		pthread_t ecoute;
		int port = atoi(argv[1]);
		if( pthread_create( &ecoute, NULL, threadEcoute, (void*)&port) < 0 ) {
	        perror("Cannot throw listening thread.\n");
	    }
	    
	    serveur = 1;
	    while(serveur) {
	        usleep(1);
	    }
	    
	    close(s_ecoute);            // arret du socket d'ecoute
	    pthread_cancel(ecoute);     // arret du thread d'ecoute
	    
		int i = 0;
		for(i = 0 ; i < maxClients ; ++i) {
			if(clients[i] != 0){
				pthread_join(clients[i], 0); // attente
			}
		}
		
	free(clients);	// liberation des threads
  }
  return 0;
}


/// Fichier serveur_ssl.c pour ipv4

SSL_CTX* InitCTX(void) {   
	const SSL_METHOD *method;
    SSL_CTX *ctx;
 
    OpenSSL_add_all_algorithms();  /* load & register all cryptos. */
    SSL_load_error_strings();   /* load all error messages */
    method = SSLv3_server_method();  
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


/*int main(int argc, char *argv[])
{   
	SSL_CTX *ctx;
	SSL *ssl;
	char service[20],host[100];
	int s_ecoute,s_com, debut=1;
	int portnum = 2000;
	struct sockaddr_in appelant;
	socklen_t len = sizeof(appelant);

	SSL_library_init();   // initialisation librairie ssl
	ctx= InitCTX();  // initialisation context SSL -> fct 
	LoadCertificates(ctx, "server.crt", "server.key"); 
	//load certs
	s_ecoute = OpenListener(portnum);
	while(debut)
	{
		s_com=accept(s_ecoute, (struct sockaddr*)&appelant,&len);
		ssl = SSL_new(ctx);   // get new ssl state with context 
		SSL_set_fd(ssl, s_com);   /// set connection socket to SSL state
		Servlet(ssl);         // coeur du progamme
	}
	close(s_ecoute);          
	SSL_CTX_free(ctx);    
 
}*/

void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile) {
    /* set the local certificate from CertFile */
if ( SSL_CTX_use_certificate_file(ctx, CertFile,SSL_FILETYPE_PEM)<=0 )
    {   ERR_print_errors_fp(stderr);
        abort(); }
    
 /* set the private key from KeyFile (may be the same as CertFile) */
if ( SSL_CTX_use_PrivateKey_file(ctx,KeyFile, SSL_FILETYPE_PEM)<=0 )
    {  ERR_print_errors_fp(stderr);
        abort();    }
    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    { fprintf(stderr, "Problem key and  public certificate\n");
        abort();}
}

int OpenListener(int port) {
	int sd;
    struct sockaddr_in addr;
 
    sd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        perror("Probleme sur le bind\n");
        abort();
    }
    listen(sd, 10);
    return sd;
}

/*void Servlet(SSL* ssl) {
	char buf[1024];
    char reply[1024];
    int sd, bytes;
    
    char *retour="Merci, mais ici tout va bien";

    if ( SSL_accept(ssl) == -1 )  // do SSL-protocol accept 
        ERR_print_errors_fp(stderr);
    else
    {
        ShowCerts(ssl);        // get any certificates du client
        bytes = SSL_read(ssl, buf, sizeof(buf)); // lecture
	printf("lecture de %d bytes\n",bytes);
        if ( bytes > 0 )
        {
            buf[bytes] = 0;
            printf("Client msg: \"%s\"\n", buf);
            sprintf(reply, retour, buf);   
            SSL_write(ssl, reply, strlen(reply)); // envoie
        }
        else
            ERR_print_errors_fp(stderr);
    }
    sd = SSL_get_fd(ssl);       // get socket connection
    SSL_free(ssl);         // release SSL state
    close(sd);          // close connection
}*/
