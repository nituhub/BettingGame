cc = gcc
CFLAGS = -Wall -g
RM = rm

all:	client.c server.c
		$(cc) $(CFLAGS) -o server server.c Betserver.c 
		$(cc) $(CFLAGS) -o client client.c Betserver.c 

clean:
	RM server client