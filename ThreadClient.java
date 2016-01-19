import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.io.Reader;
import java.net.Socket;

public class ThreadClient extends Thread {
	Socket client;

	private class Lettre {
		public long v;
		public long c;
	}

	public ThreadClient(Socket c) {
		client = c;
	}

	/*
	 *
	 *	Compte le nombre de voyelles et de consonnes
	 *
	 */
	Lettre countLetters(String s) {
		Lettre l = new Lettre();

		l.v = 0;
		l.c = 0;

		for(int i = 0 ; i < s.length() ; ++i) {
			if(Character.isLetter(s.charAt(i))) {
				if("eaiouyEAIOUY".contains(""+(s.charAt(i)))) {
					(l.v)++;
				} else {
					(l.c)++;
				}
			}
		}

		return l;
	}

	public void run() {
		Reader reader;
		PrintStream sortie=null;
		String line;
		try {
			reader = new InputStreamReader(client.getInputStream());
			sortie = new PrintStream(client.getOutputStream());
			BufferedReader keyboard = new BufferedReader (reader);
			StringBuffer response = new StringBuffer();

			line = keyboard.readLine();
			System.out.println("Client's request:"+ '\n' +">>> "+ line);

			Lettre l = countLetters(line);

			if(line.charAt(0)=='+') {
				response.append("Number of consonants: "+l.c);
			} else if(line.charAt(0)=='-') {
				response.append("Number of vowels: "+l.v);
			} else {
				response.append("Unknown request!\n");
			}

			response.append('\0');
			sortie.println(response);
			System.out.println("Server's response:"+'\n'+">>> "+response);
			System.out.println("Connexion closed.");
			client.close();

		} catch(Exception e) {
			e.printStackTrace();
		}
	}
}