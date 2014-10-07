CC=gcc
CFLAGS=-w -g

router: rout.o
	$(CC) $(CFLAGS) -o router rout.o  -I.

.PHONY : KillAll clean debug

k:
	killall -9 router

clean: 
	rm rout.o 
