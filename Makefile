# definition des cibles particulieres
.PHONY: clean

# definition des variables
CXX = gcc
CXXFLAGS = -O2 -Wall -Wextra -g

all : client serveur clean

client : client.c
	$(CXX) $^ -o ./$@ $(CXXFLAGS)

serveur : serveur.c
	$(CXX) $^ -o ./$@ $(CXXFLAGS)

clean :
	rm -rf *.bak; rm -rf *.o
