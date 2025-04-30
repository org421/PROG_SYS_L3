CC = gcc
CFLAGS = -Wall

all: initial archiviste journaliste

initial: initial.c 
	$(CC) $(CFLAGS) initial.c -o initial

archiviste: archiviste.c 
	$(CC) $(CFLAGS) archiviste.c -o archiviste

journaliste: journaliste.c 
	$(CC) $(CFLAGS) journaliste.c -o journaliste

clean:
	rm -f initial journaliste archiviste
