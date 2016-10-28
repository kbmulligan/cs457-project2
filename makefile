CC = "g++"

all: core ss awget

core: core.cpp
	$(CC) -Wall -Werror -std=c++11 -pthread -c core.cpp

awget: awget.cpp
	$(CC) -Wall -Werror -std=c++11 -pthread -o awget awget.cpp core.cpp

ss: ss.cpp
	$(CC) -Wall -Werror -std=c++11 -pthread -o ss ss.cpp core.cpp

clean:
	rm core.o
