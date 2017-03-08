all:
	gcc -Wall -pedantic -ansi -lcrypto src/main.c src/encryption.c src/server.c -o main

install:
	cp src/main ../../../bin/

clean:
	rm main
