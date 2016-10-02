CC = "g++"

all: ss

ss: ss.cpp
	g++ -Wall -Werror -o ss ss.cpp

clean:
	rm ss.o
