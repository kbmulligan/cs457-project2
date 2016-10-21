CC = "g++"

all: ss

ss: ss.cpp
	g++ -Wall -Werror -std=c++11 -pthread -o ss ss.cpp

clean:
	rm ss.o
