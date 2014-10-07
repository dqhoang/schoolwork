CC=gcc
CFLAGS=-w -g

rout: rout.o
	$(CC) $(CFLAGS) -o rout rout.o  -I.

.PHONY : KillAll clean debug

KillAll:
	killall -9 rout

clean: 
	rm rout.o 
