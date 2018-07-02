client: peer.o client.o m.o
	gcc peer.o client.o m.o -o client -lpthread -Wall -g

peer.o: ../peer.h ../peer.c
	gcc -c ../peer.c -o peer.o

client.o: client.c 
	gcc -c client.c -o client.o

m.o: ../m.c
	gcc -c ../m.c -o m.o
clean:
	rm -rf *.o client
