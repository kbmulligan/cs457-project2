CC = g++
CFLAGS = -Wall -Werror -std=c++11

all: core.o ss awget


awget: awget.cpp core.cpp
	$(CC) $(CLFAGS) -std=c++11 -pthread core.cpp awget.cpp -o awget 

ss: ss.cpp core.cpp
	$(CC) $(CLFAGS) -std=c++11 -pthread core.cpp ss.cpp -o ss 

core.o: core.cpp
	$(CC) $(CLFAGS) -std=c++11 -c core.cpp

clean:
	rm core.o
