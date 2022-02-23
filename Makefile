CPPFLAG=-pedantic -Wall -Werror
TARGETS=ringmaster player

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

player: player.cpp player.h socket.h socket.o potato.h
	g++ -g -o $@ $<

ringmaster: ringmaster.cpp ringmaster.h socket.h socket.o potato.h
	g++ -g -o $@ $<

socket.o: socket.cpp socket.h
	g++ $(CPPFLAG) -c $<
