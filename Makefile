# definition des cibles particulieres
.PHONY: clean

# definition des variables
CXX = gcc
CXXFLAGS = -O2 -Wall -Wextra -g -lpthread

all : client serveur serveurJava clientSecu serveurSecu clean

client : client.c
	$(CXX) $^ -o ./$@ $(CXXFLAGS)
	
serveur : serveur.c
	$(CXX) $^ -o ./$@ $(CXXFLAGS)

serveurJava : JServeur.java ThreadClient.java
	rm -rf classes; mkdir classes
	javac -d classes JServeur.java

clientSecu : clientSecu.c
	$(CXX) $^ -o  ./$@ $(CXXFLAGS) -lssl -lcrypto
	
serveurSecu : serveurSecu.c
	$(CXX) $^ -o ./$@ $(CXXFLAGS) -lssl -lcrypto

clean :
	rm -rf *.bak; rm -rf *.o; rm -rf *~
	
wipe : clean
	rm client clientSecu serveur serveurSecu; rm -rf classes
