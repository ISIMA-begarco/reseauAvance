# definition des cibles particulieres
.PHONY: clean

# definition des variables
CXX = gcc
CXXFLAGS = -O2 -Wall -Wextra -g -lpthread

all : client serveur serveurJava clean

client : client.c
	$(CXX) $^ -o ./$@ $(CXXFLAGS)
	
serveur : serveur.c
	$(CXX) $^ -o ./$@ $(CXXFLAGS)

serveurJava : JServeur.java ThreadClient.java
	mkdir classes
	javac -d classes JServeur.java

clean :
	rm -rf *.bak; rm -rf *.o; rm -rf *~
	
wipe : clean
	rm client serveur; rm -rf classes
