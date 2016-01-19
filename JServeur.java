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
		System.out.println("Server on.");
		while (true) {
		    soc = s.accept();
		    System.out.println("Connexion realised to " + soc.toString());

		    /// Lancement d'un nouveau fred
		    ThreadClient t = new ThreadClient(soc);
		    t.run();
		}
	}
}
