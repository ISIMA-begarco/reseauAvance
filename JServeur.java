import java.io.*;
import java.net.*;

public class JServeur {
	private static int port;
	public static void main (String [] args) throws Exception {
		Socket soc;
		if(args.length != 1) {
			System.out.println("usage : java serveur port");
			System.exit(0);
		}
		port = new Integer(args[0]).intValue();
		ServerSocket s = new ServerSocket (port);
		System.out.println("La socket serveur est cree");
		while (true) {
		    soc = s.accept();
		    System.out.println("Connexion realise a " + soc.toString());
		    
		    /// Lancement d'un nouveau fred
		    ThreadClient t = new ThreadClient(soc);
		    t.run();
		}
	}
}


/**
Creation du certificat ssl
1- generation de la cle privee :
	openssl genrsa -out server.key 1024 ou 2048
2- demande de signature
	openssl req -new -key server.key -out server.csr
3- auto signe
	openssl x509 -req -days 365 -in server.csr -signkey server.key - out server.crt

4- le serveur a besoin de server.key et server.crt

5- pour compiler -lssl et -lcrypto
**/