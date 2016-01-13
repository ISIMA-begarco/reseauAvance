# definition des cibles particulieres
.PHONY: clean

# definition des variables
CXX = gcc
CXXFLAGS = -O2 -Wall -Wextra -g -lpthread

all : client serveur client_http clean

client : client.c
	$(CXX) $^ -o ./$@ $(CXXFLAGS)
	
client_http : client_http.c
	$(CXX) $^ -o ./$@ $(CXXFLAGS)


serveur : serveur.c
	$(CXX) $^ -o ./$@ $(CXXFLAGS)

clean :
	rm -rf *.bak; rm -rf *.o
