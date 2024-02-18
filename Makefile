all: client server circb

circb: circb.o
	gcc -I ${INCLUDEDIR} -o circb circb.o

circb.o: src/circb.c
	gcc -I ${INCLUDEDIR} -c src/circb.c

server: server.o
	gcc -I ${INCLUDEDIR} -o server server.o

server.o: src/components/server.c
	gcc -I ${INCLUDEDIR} -c src/components/server.c

client: client.o
	gcc -I ${INCLUDEDIR} -o client client.o

client.o: src/components/client.c
	gcc -I ${INCLUDEDIR} -c src/components/client.c
     
clean:
	rm server client server.o client.o