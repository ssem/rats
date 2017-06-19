all:
	gcc -Wall -pedantic -ansi -lcrypto src/main.c src/server.c src/packet_parser/packet_parser.c -o main

install:
	cp src/main ../../../bin/

clean:
	rm main
