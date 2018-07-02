server: event.o peer.o m.o sudoku.o server.o
	gcc event.o peer.o m.o sudoku.o server.o -o sudoku -Wall -g

event.o: event.h event.c event_select.c event_epoll.c event_poll.c
	gcc -c event.c -o event.o

peer.o: ../peer.h ../peer.c
	gcc -c ../peer.c -o peer.o

m.o: ../m.c
	gcc -c ../m.c -o m.o

sudoku.o: sudoku.c
	gcc -c sudoku.c -o sudoku.o

server.o: server.c
	gcc -c server.c -o server.o

clean:
	rm -rf *.o sudoku
